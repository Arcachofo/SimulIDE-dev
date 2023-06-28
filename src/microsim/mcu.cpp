/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>
#include <QFileInfo>
#include <QTranslator>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "mcu.h"
#include "cpubase.h"
#include "mcuport.h"
#include "mcupin.h"
#include "mcuwdt.h"
#include "mcucreator.h"
#include "scriptcpu.h"
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "componentselector.h"
#include "mcumonitor.h"
#include "memdata.h"
#include "mcuuart.h"
#include "mcuintosc.h"
#include "utils.h"
#include "watcher.h"
#include "linkable.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"
#include "boolprop.h"

#include "propdialog.h"

Mcu* Mcu::m_pSelf = NULL;

LibraryItem* Mcu::libraryItem()
{
    return new LibraryItem(
        "NEW_MCU",
        "",
        "ic2.png",
        "MCU",
        Mcu::construct );
}

Component* Mcu::construct( QObject* parent, QString type, QString id )
{
    Mcu* mcu = new Mcu( parent, type, id );
    if( !m_error) m_error = McuCreator::createMcu( mcu, id );
    if( !m_error) mcu->setLogicSymbol( false );

    if( m_error > 0 )
    {
        Circuit::self()->removeComp( mcu );
        mcu = NULL;
        m_pSelf = NULL;
        m_error = 0;
    }
    return mcu;
}

Mcu::Mcu( QObject* parent, QString type, QString id )
   : Chip( parent, type, id )
   , m_eMcu( this, id )
{
    qDebug() << "        Initializing"<<id;

    addPropGroup( { tr("Main"), {},0} );

    m_device = m_name;//.split("_").last(); // for example: "atmega328-1" to: "atmega328"
    if( m_device.contains("_") ) m_device = m_device.split("_").last(); // MCU in Subcircuit

    if( m_device.startsWith("p") ) // PICs TODELETE
    {
        if( m_device.endsWith("a") ) m_device.remove( m_device.size()-1, 1 );
        m_device.replace("f", "F");
    }
    setName( m_device );

    m_resetPin   = NULL;
    m_portRstPin = NULL;
    m_mcuMonitor = NULL;
    m_scriptLink = NULL;

    m_autoLoad = false;
    m_scripted = false;
    m_resetPol = false;
    m_linkable = true;

    m_serialMon = -1;

    m_icColor = QColor( 20, 30, 60 );

    QString xmlFile = ComponentSelector::self()->getXmlFile( m_device );
    QFile file( xmlFile );

    if( file.exists() )
    {
        QDomDocument domDoc = fileToDomDoc( xmlFile, "Mcu::Mcu" );
        if( domDoc.isNull() ) { m_error = 1; return; }

        QDomElement root  = domDoc.documentElement();
        QDomNode    rNode = root.firstChild();

        bool found = false;
        while( !rNode.isNull() )
        {
            QDomElement itemSet = rNode.toElement();
            QDomNode    node    = itemSet.firstChild();

            QString folder = "";
            if( itemSet.hasAttribute("folder") ) folder = itemSet.attribute("folder");

            while( !node.isNull() )
            {
                QDomElement element = node.toElement();
                if( element.attribute("name") == m_device )
                {
                    if( element.hasAttribute("folder") ) folder = element.attribute("folder");

                    QFileInfo fi( xmlFile );
                    if( !folder.isEmpty() )
                    {
                        QString stripped = fi.absolutePath()+"/"+folder+"/"+m_device+"/"+m_device;
                        m_pkgeFile = stripped+".package";
                        m_dataFile = stripped+".mcu";
                    }else{
                        m_pkgeFile = fi.absolutePath()+"/"+ element.attribute("package")+".package";
                        m_dataFile = fi.absolutePath()+"/"+ element.attribute("data")+".mcu";
                    }
                    found = true;
                }
                if( found ) break;
                node = node.nextSibling();
            }
            if( found ) break;
            rNode = rNode.nextSibling();
        }
    }else{
        QDir mcuDir;
        QString folder = ComponentSelector::self()->getFileDir( m_device );

        if( !folder.isEmpty() ) // Found in folder (no xml file)
        {
            mcuDir = QDir( folder );
        }
        else              // Try to find a "data" folder in Circuit folder
        {
            mcuDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
            folder = "data/"+m_device;
        }
        m_dataFile = mcuDir.absoluteFilePath( folder+"/"+m_device+".mcu" );
        m_pkgeFile = mcuDir.absoluteFilePath( folder+"/"+m_device+".package" );

        QFile dataFile( m_dataFile );
        QFile pkgeFile( m_pkgeFile );
        if( !dataFile.exists() || !pkgeFile.exists() )
        {
            MessageBoxNB( "Mcu::Mcu", "                               \n"+
                      tr("Files not found for: %1").arg( m_device ) );
            m_error = 1;
            return;
        }
    }

    QSettings* settings = MainWindow::self()->settings();
    m_lastFirmDir = settings->value("lastFirmDir").toString();

    QDir dir( m_lastFirmDir );
    if( !dir.exists() ) m_lastFirmDir = Circuit::self()->getFilePath();

    Simulator::self()->addToUpdateList( this );

    qDebug() << "       "<<id<< "Initialized"<<endl;
}

void Mcu::setup( QString type )
{
    if     ( type == "iou" ) m_deviceType = typeIOU;
    else if( type == "mpu" ) m_deviceType = typeMPU;
    else if( type == "mcu" ) m_deviceType = typeMCU;
    else                     m_deviceType = typeNONE;

    //if( m_deviceType >= typeMPU )
        slotmain(); //m_pSelf = this;

    if( m_deviceType == typeMCU )
    {
    addPropGroup( { tr("Main"), {
new DoubProp<Mcu>("Frequency", tr("Frequency")    ,"MHz", this, &Mcu::extFreq,    &Mcu::setExtFreq ),
new StrProp <Mcu>("Program"  , tr("Firmware")        ,"", this, &Mcu::program,    &Mcu::setProgram ),
new BoolProp<Mcu>("Auto_Load", tr("Reload hex at \
                                   Simulation Start"),"", this, &Mcu::autoLoad,   &Mcu::setAutoLoad ),
new BoolProp<Mcu>("saveEepr" , tr("EEPROM persitent"),"", this, &Mcu::saveEepr,   &Mcu::setSaveEepr ),
new BoolProp<Mcu>("Logic_Symbol", tr("Logic Symbol") ,"", this, &Mcu::logicSymbol,&Mcu::setLogicSymbol, propNoCopy )
    }, 0} );

    addPropGroup( { tr("Config"), {
new ComProperty( "", tr("Changes applied after Simulation Restart"),"","",0),
                    }, groupNoCopy} );

if( m_portRstPin )
addProperty(tr("Config"),new BoolProp<Mcu>("Rst_enabled", tr("Enable Reset Pin")   ,"", this, &Mcu::rstPinEnabled, &Mcu::enableRstPin ) );

if( m_eMcu.m_intOsc && m_eMcu.m_intOsc->clkInPin() )
addProperty(tr("Config"),new BoolProp<Mcu>("Ext_Osc"    , tr("External Oscillator"),"", this, &Mcu::extOscEnabled, &Mcu::enableExtOsc ) );

if( m_eMcu.m_wdt )
addProperty(tr("Config"),new BoolProp<Mcu>("Wdt_enabled", tr("Enable WatchDog")    ,"", this, &Mcu::wdtEnabled   , &Mcu::enableWdt ) );

if( m_eMcu.m_intOsc && m_eMcu.m_intOsc->clkOutPin() )
addProperty(tr("Config"),new BoolProp<Mcu>("Clk_Out"    , tr("Clock Out")          ,"", this, &Mcu::clockOut     , &Mcu::setClockOut ) );

    addPropGroup( {"Hidden", {
new StrProp<Mcu>("varList"  ,"","", this, &Mcu::varList,   &Mcu::setVarList),
new StrProp<Mcu>("cpuRegs"  ,"","", this, &Mcu::cpuRegs,   &Mcu::setCpuRegs),
new StrProp<Mcu>("eeprom"   ,"","", this, &Mcu::getEeprom, &Mcu::setEeprom ),
new IntProp<Mcu>("SerialMon","","", this, &Mcu::serialMon, &Mcu::setSerialMon )
    }, groupHidden } );
    }
    else //if( m_deviceType == typeMPU )
    {
//addProperty(tr("Main"),new DoubProp<Mcu>( "Frequency"   , tr("Frequency"),"MHz" , this, &Mcu::freq,    &Mcu::setFreq ) );
addProperty(tr("Main"),new BoolProp<Mcu>( "Logic_Symbol", tr("Logic Symbol"),"", this, &Mcu::logicSymbol, &Mcu::setLogicSymbol ) );

    addPropGroup( {"Hidden", {
new StrProp<Mcu>("varList", "","", this, &Mcu::varList,   &Mcu::setVarList),
new StrProp<Mcu>("cpuRegs", "","", this, &Mcu::cpuRegs,   &Mcu::setCpuRegs),
new StrProp<Mcu>("Links"  , "","", this, &Mcu::getLinks , &Mcu::setLinks )
    }, groupHidden } );
    }
}
Mcu::~Mcu()
{
    if( m_mcuMonitor ) delete m_mcuMonitor;
    if( m_pSelf == this ) m_pSelf= NULL;
}

bool Mcu::setPropStr( QString prop, QString val )
{
    if( prop =="program" ) setProgram( val ); //  Old: TODELETE
    else if( prop =="Mhz" ) setFreq( val.toDouble()*1e6 );
    else return Component::setPropStr( prop, val );
    return true;
}

void Mcu::initialize()
{
    m_crashed = false;
    if( m_mcuMonitor ) m_mcuMonitor->updateRamTable();
}

void Mcu::stamp()
{
    if( m_resetPin ){
        m_resetPin->changeCallBack( this );

        if( m_resetPin == m_portRstPin ) // Not dedicated Reset Pin
        {
            m_portRstPin->controlPin( true, true );
            m_portRstPin->setPinMode( input );
        }
        m_resetPin->warning( !m_resetPin->connector() );
    }
    else m_eMcu.hardReset( false );

    if( m_autoLoad )
    { if( !m_eMcu.m_firmware.isEmpty() ) load( m_eMcu.m_firmware ); }
}

void Mcu::updateStep()
{
    if( m_crashed )
    {
        Simulator::self()->setWarning( /*m_warning*/0 );
        update();
    }
    if( m_mcuMonitor
     && m_mcuMonitor->isVisible() ) m_mcuMonitor->updateStep();

    m_eMcu.cpu->updateStep();
}

void Mcu::voltChanged() // Reset Pin callBack
{
    m_eMcu.hardReset( m_resetPin->getInpState() == m_resetPol );
}

void Mcu::setProgram( QString pro )
{
    if( pro == "" ) return;
    if( Circuit::self()->isSubc() ) m_eMcu.m_firmware = pro; // Let Subcircuit load firmware with path to subc dir
    else load( pro );
}

QString Mcu::varList()
{ return m_eMcu.getRamTable()->getVarSet().join(","); }

void Mcu::setVarList( QString vl )
{
    if( vl.isEmpty() ) return;
    QStringList vars = vl.split(",");
    QStringList varSet;
    for( QString str : vars ) // Format: {"var0/aadr0/type0", "var1/addr1/type1",...}
    {
        QStringList words = str.split("/");
        if( words.size() < 1 ) continue;
        QString name = words.at(0);
        int addr = -1;
        QString type = "uint8";
        bool ok = false;
        if( words.size() > 1 ) addr = words.at(1).toInt( &ok, 0 );
        if( words.size() > 2 ) type = words.at(2);
        if( ok ) m_eMcu.getRamTable()->addVariable( name, addr, type );
        varSet.append( name );
    }
    m_eMcu.getRamTable()->loadVarSet( varSet );
}
//{ m_eMcu.getRamTable()->loadVarSet( vl.split(",") ); }

QString Mcu::cpuRegs()
{ return m_eMcu.getCpuTable()->getVarSet().join(","); }

void Mcu::setCpuRegs( QString vl )
{ m_eMcu.getCpuTable()->loadVarSet( vl.split(",") ); }

void Mcu::setEeprom( QString eep )
{
    if( eep.isEmpty() ) return;
    QVector<int> eeprom;
    QStringList list = eep.split(",");
    for( QString val : list ) eeprom.append( val.toUInt() );

    if( eeprom.size() > 0 ) m_eMcu.setEeprom( &eeprom );
}

QString Mcu::getEeprom()  // Used by property, stripped to last written value.
{
    QString eeprom;
    if( m_eMcu.m_saveEepr )
    {
        int size = m_eMcu.romSize();
        if( size > 0 )
        {
            bool empty = true;
            for( int i=size-1; i>=0; --i )
            {
                uint8_t val = m_eMcu.getRomValue( i );
                if( val < 0xFF ) empty = false;
                if( empty ) continue;
                eeprom.prepend( QString::number( val )+"," );
    }   }   }
    return eeprom;
}

void Mcu::loadEEPROM()
{
   QVector<int>* eeprom = m_eMcu.eeprom();
   MemData::loadData( eeprom, false );
   m_eMcu.setEeprom( eeprom );
   if( m_mcuMonitor ) m_mcuMonitor->tabChanged( 1 );
}

void Mcu::saveEEPROM() { MemData::saveData( m_eMcu.eeprom() ); }

void Mcu::slotLoad()
{
    QDir dir( m_lastFirmDir );
    if( !dir.exists() ) m_lastFirmDir = Circuit::self()->getFilePath();

    QString fileName = QFileDialog::getOpenFileName( NULL, tr("Load Firmware"), m_lastFirmDir,
                       tr("All files (*.*);;Hex Files (*.hex)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    load( fileName );
}

void Mcu::slotReload()
{
    if( !m_eMcu.m_firmware.isEmpty() ) load( m_eMcu.m_firmware );
    else QMessageBox::warning( 0, "Mcu::slotReload", tr("No File to reload ") );
}

bool Mcu::load( QString fileName )
{
    if( fileName.isEmpty() ) return false;

    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString fileNameAbs  = circuitDir.absoluteFilePath( fileName );
    QString cleanPathAbs = circuitDir.cleanPath( fileNameAbs );

    if( !QFileInfo::exists( cleanPathAbs ) )
    {
        qDebug() << "Error: file doesn't exist:\n"<<cleanPathAbs<<"\n";
        return false;
    }
    if( Simulator::self()->simState() > SIM_STARTING )  CircuitWidget::self()->powerCircOff();

    int size = m_eMcu.flashSize();
    QVector<int> pgm( size );
    for( int i=0; i<size; ++i ) pgm[i] = m_eMcu.getFlashValue( i );

    if( !MemData::loadFile( &pgm, cleanPathAbs, false, m_eMcu.m_wordSize*8, &m_eMcu ) )
        return false;

    for( int i=0; i<size; ++i ) m_eMcu.setFlashValue( i, pgm.at(i) );
    qDebug() << "Firmware succesfully loaded\n";

    m_eMcu.m_firmware = circuitDir.relativeFilePath( cleanPathAbs );
    m_lastFirmDir = QFileInfo( cleanPathAbs ).absolutePath();
    if( m_propDialog ) m_propDialog->updtValues();

    QSettings* settings = MainWindow::self()->settings();
    settings->setValue( "lastFirmDir", circuitDir.relativeFilePath( fileName ) );

    return true;
}

void Mcu::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else{
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

void Mcu::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    //if( m_deviceType >= typeMPU )
    {
        QAction* mainAction = menu->addAction( QIcon(":/subc.png"),tr("Main Mcu") );
        connect( mainAction, &QAction::triggered,
                       this, &Mcu::slotmain, Qt::UniqueConnection );
    }

    if( m_scriptLink )
    {
        QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
        connect( linkCompAction, &QAction::triggered,
                           this, &Mcu::slotLinkComp, Qt::UniqueConnection );
    }

    if( m_deviceType == typeMCU )
    {
        QAction* loadAction = menu->addAction( QIcon(":/load.svg"),tr("Load firmware") );
        connect( loadAction, &QAction::triggered,
                       this, &Mcu::slotLoad, Qt::UniqueConnection );

        QAction* reloadAction = menu->addAction( QIcon(":/reload.svg"),tr("Reload firmware") );
        connect( reloadAction, &QAction::triggered,
                         this, &Mcu::slotReload, Qt::UniqueConnection );

        menu->addSeparator();

        QAction* loadDaAction = menu->addAction( QIcon(":/open.png"),tr("Load EEPROM data from file") );
        connect( loadDaAction, &QAction::triggered,
                         this, &Mcu::loadEEPROM, Qt::UniqueConnection );

        QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save EEPROM data to file") );
        connect( saveDaAction, &QAction::triggered,
                         this, &Mcu::saveEEPROM, Qt::UniqueConnection );
    }
    //if( m_deviceType >= typeMPU )
    {
        menu->addSeparator();

        QAction* openRamTab = menu->addAction( QIcon(":/terminal.svg"),tr("Open Mcu Monitor.") );
        connect( openRamTab, &QAction::triggered,
                       this, &Mcu::slotOpenMcuMonitor, Qt::UniqueConnection );
    }
    if( m_deviceType == typeMCU )
    {
        QMenu* serMonMenu = menu->addMenu( tr("Open Serial Monitor.") );

        QSignalMapper* sm = new QSignalMapper();
        for( uint i=0; i<m_eMcu.m_usarts.size(); ++i )
        {
            QAction* openSerMonAct = serMonMenu->addAction( "USart"+QString::number(i+1) );
            connect( openSerMonAct, &QAction::triggered, sm, QOverload<>::of(&QSignalMapper::map) );
            sm->setMapping( openSerMonAct, i+1 );
        }
        connect( sm, QOverload<int>::of(&QSignalMapper::mapped), this, &Mcu::slotOpenTerm );
    }
    menu->addSeparator();

    /*if( !event )
    {
        QAction* propertiesAction = menu->addAction( QIcon( ":/properties.svg"),tr("Properties") );
        connect( propertiesAction, &QAction::triggered,
                             this, &Mcu::slotProperties, Qt::UniqueConnection );
        menu->addSeparator();
    }*/
}

void Mcu::slotmain()
{
    m_pSelf = this;
    m_eMcu.setMain();
    Circuit::self()->update();
}

void Mcu::slotLinkComp()
{
    Linkable::startLinking();
}

void Mcu::slotOpenMcuMonitor()
{
    if( !m_mcuMonitor )
    {
        m_mcuMonitor = new MCUMonitor( CircuitWidget::self(), &m_eMcu );
        m_mcuMonitor->setWindowTitle( idLabel() );
    }
    m_mcuMonitor->show();
}

void Mcu::slotOpenTerm( int num )
{
    m_eMcu.m_usarts.at(num-1)->openMonitor( idLabel(), num );
    m_serialMon = num;
}

int Mcu::serialMon()
{
    if( m_serialMon < 0 ) return -1;
    if( m_eMcu.m_usarts.at( m_serialMon-1 )->serialMon() ) return m_serialMon;
    return -1;
}

void Mcu::setSerialMon( int s ) { if( s>=0 ) slotOpenTerm( s ); }

void Mcu::setLinkedVal( int index, int v, int i )
{
    if( index >= m_linkedComp.size() ) return;
    m_linkedComp.at( index )->setLinkedValue( v, i );
}

void Mcu::setLinkedStr( int index, QString str, int i )
{
    if( index >= m_linkedComp.size() ) return;
    m_linkedComp.at( index )->setLinkedString( str, i );
}

void Mcu::setLinkedValue( int v, int i )
{
    if( m_scriptLink ) m_scriptLink->setLinkedVal( v, i );
}

void Mcu::setLinkedString( QString str, int i )
{
    if( m_scriptLink ) m_scriptLink->setLinkedStr( str, i );
}

Pin* Mcu::addPin( QString id, QString type, QString label,
                  int pos, int xpos, int ypos, int angle, int length )
{
    IoPin* pin = NULL;
    if( type.contains("rst") )
    {
        if( !m_resetPin )
            m_resetPin = new IoPin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this, input );
        pin = m_resetPin;
        m_resetPol = type.startsWith("!") ;
    }
    else pin = m_eMcu.getIoPin( id ); // I/O Port

    if( !pin ) return NULL;

    QColor color = Qt::black;
    if( !m_isLS ) color = QColor( 250, 250, 200 );

    if( type.startsWith("nul") )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
    }
    else if( type.startsWith("inv") ) pin->setInverted( true );

    pin->setPos( QPoint( xpos, ypos ) );
    pin->setPinAngle( angle );
    pin->setLength( length );
    pin->setLabelText( label );
    pin->setLabelColor( color );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    return pin;
}

bool Mcu::rstPinEnabled()
{
    if( !m_portRstPin ) return true;
    return (m_resetPin == m_portRstPin);
}

void Mcu::enableRstPin( bool en )
{
    if( !m_portRstPin ) return;

    m_portRstPin->controlPin( en, en );

    if( en ){
        m_resetPin = m_portRstPin;
        m_portRstPin->setPinMode( input );
    }
    else m_resetPin = NULL;
}

bool Mcu::extOscEnabled()
{
    if( m_eMcu.m_intOsc ) return m_eMcu.m_intOsc->extClock();
    return false;
}

void Mcu::enableExtOsc( bool en )
{
    if( m_eMcu.m_intOsc ) m_eMcu.m_intOsc->enableExtOsc( en );
}

bool Mcu::wdtEnabled()
{
    if( m_eMcu.m_wdt ) return m_eMcu.m_wdt->enabled();
    return false;
}

void Mcu::enableWdt( bool en ) { if( m_eMcu.m_wdt ) m_eMcu.m_wdt->enable( en ); }

bool Mcu::clockOut()
{
    if( m_eMcu.m_intOsc ) return m_eMcu.m_intOsc->clockOut();
    return false;
}

void Mcu::setClockOut( bool clkOut ) { if( m_eMcu.m_intOsc ) m_eMcu.m_intOsc->setClockOut( clkOut ); }

QStringList Mcu::getEnumUids( QString e) { return m_eMcu.cpu->getEnumUids( e ); }
QStringList Mcu::getEnumNames( QString e) { return m_eMcu.cpu->getEnumNames( e ); }

void Mcu::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );

    if( m_pSelf == this )
    {
        QPen pen( Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p->setPen( pen );
        p->setBrush( Qt::yellow );
        if( m_width == m_height ) p->drawRoundedRect( 4, 4, 4, 4 , 2, 2);
        else                      p->drawRoundedRect( m_area.width()/2-2, -1, 4, 4 , 2, 2);
}   }

