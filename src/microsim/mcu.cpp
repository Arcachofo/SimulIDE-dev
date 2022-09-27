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
#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "componentselector.h"
#include "mcumonitor.h"
#include "memdata.h"
#include "mcuuart.h"
#include "utils.h"

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

    addPropGroup( { tr("Main"), {}} );

    m_device = m_name;//.split("_").last(); // for example: "atmega328-1" to: "atmega328"
    if( m_device.contains("_") ) m_device = m_device.split("_").last(); // MCU in Subcircuit

    if( m_device.startsWith("p") ) // PICs TODELETE
    {
        if( m_device.endsWith("a") ) m_device.remove( m_device.size()-1, 1 );
        m_device.replace("f", "F");
    }
    setName( m_device );

    m_clkPin[0]  = NULL;
    m_clkPin[1]  = NULL;
    m_resetPin   = NULL;
    m_portRstPin  = NULL;
    m_mcuMonitor = NULL;
    m_autoLoad   = false;
    m_extClock   = false;

    m_serialMon = -1;

    m_icColor = QColor( 20, 30, 60 );

    QString xmlFile = ComponentSelector::self()->getXmlFile( m_device );
    QFile file( xmlFile );

    if(( xmlFile == "" ) || ( !file.exists() ))
    {
        MessageBoxNB( "Mcu::Mcu", "                               \n"+
                  tr("xml file not found for: %1").arg( m_device ) );
        m_error = 1;
        return;
    }
    QDomDocument domDoc = fileToDomDoc( xmlFile, "Mcu::Mcu" );
    if( domDoc.isNull() ) { m_error = 1; return; }

    QDomElement root  = domDoc.documentElement();
    QDomNode    rNode = root.firstChild();

    while( !rNode.isNull() )
    {
        QDomElement element = rNode.toElement();
        QDomNode    node    = element.firstChild();

        while( !node.isNull() )
        {
            QDomElement element = node.toElement();
            if( element.attribute("name") == m_device )
            {
                // Get package file
                QDir dataDir( xmlFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                m_pkgeFile = dataDir.filePath( element.attribute( "package" ) )+".package";

                // Get data file
                m_dataFile = dataDir.filePath( element.attribute( "data" ) )+".mcu";
                break;
            }
            node = node.nextSibling();
        }
        rNode = rNode.nextSibling();
    }
    QSettings* settings = MainWindow::self()->settings();
    m_lastFirmDir = settings->value("lastFirmDir").toString();

    QDir dir( m_lastFirmDir );
    if( !dir.exists() ) m_lastFirmDir = Circuit::self()->getFilePath();

    m_subcDir = "";

    Simulator::self()->addToUpdateList( this );

    qDebug() << "        "<<id<< "Initialized:"<<freq()*1e-6<<"MHz\n";
}

void Mcu::setup( QString type )
{
    if     ( type == "iou" ) m_deviceType = typeIOU;
    else if( type == "mpu" ) m_deviceType = typeMPU;
    else if( type == "mcu" ) m_deviceType = typeMCU;
    else                     m_deviceType = typeNONE;

    if( m_deviceType >= typeMPU ) slotmain(); //m_pSelf = this;

    if( m_deviceType == typeMCU )
    {
    addPropGroup( { tr("Main"), {
new DoubProp  <Mcu>("Frequency", tr("Frequency"),"MHz" , this, &Mcu::freq,    &Mcu::setFreq ),
new StringProp<Mcu>("Program"  , tr("Firmware")  ,""   , this, &Mcu::program, &Mcu::setProgram ),
new BoolProp  <Mcu>("Auto_Load", tr("Reload hex at Simulation Start"),"", this, &Mcu::autoLoad, &Mcu::setAutoLoad ),
new BoolProp  <Mcu>("Logic_Symbol", tr("Logic Symbol"),"", this, &Mcu::logicSymbol, &Mcu::setLogicSymbol )
    }} );
    addPropGroup( { tr("Config"), {
new BoolProp  <Mcu>( "Rst_enabled", tr("Enable Reset Pin")   ,"", this, &Mcu::rstPinEnabled, &Mcu::enableRstPin ),
new BoolProp  <Mcu>( "Ext_Osc"    , tr("External Oscillator"),"", this, &Mcu::extOscEnabled, &Mcu::enableExtOsc ),
new BoolProp  <Mcu>( "Wdt_enabled", tr("Enable WatchDog")    ,"", this, &Mcu::wdtEnabled,    &Mcu::enableWdt )
    }} );
    addPropGroup( {"Hidden", {
new StringProp<Mcu>("varList", "","", this, &Mcu::varList,   &Mcu::setVarList),
new StringProp<Mcu>("cpuRegs", "","", this, &Mcu::cpuRegs,   &Mcu::setCpuRegs),
new StringProp<Mcu>("eeprom" , "","", this, &Mcu::getEeprom, &Mcu::setEeprom ),
new IntProp   <Mcu>("SerialMon","","", this, &Mcu::serialMon, &Mcu::setSerialMon )
    }} );
    }
    else //if( m_deviceType == typeMPU )
    {
//addProperty(tr("Main"),new DoubProp<Mcu>( "Frequency"   , tr("Frequency"),"MHz" , this, &Mcu::freq,    &Mcu::setFreq ) );
addProperty(tr("Main"),new BoolProp<Mcu>( "Logic_Symbol", tr("Logic Symbol"),"", this, &Mcu::logicSymbol, &Mcu::setLogicSymbol ) );

    addPropGroup( {"Hidden", {
new StringProp<Mcu>( "cpuRegs", "","", this, &Mcu::cpuRegs,   &Mcu::setCpuRegs),
    }} );
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
}

void Mcu::voltChanged() // Reset Pin callBack
{
    m_eMcu.hardReset( !m_resetPin->getInpState() );
}

void Mcu::setProgram( QString pro )
{
    if( pro == "" ) return;
    m_eMcu.m_firmware = pro;

    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( m_eMcu.m_firmware );

    if( QFileInfo::exists( fileNameAbs ) )
    { load( m_eMcu.m_firmware ); }
}

QString Mcu::varList()
{ return m_eMcu.getRamTable()->getVarSet().join(","); }

void Mcu::setVarList( QString vl )
{ m_eMcu.getRamTable()->loadVarSet( vl.split(",") ); }

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
    }   }
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

void Mcu::remove()
{
    for( Pin* pin : m_pinList ) pin->removeConnector();
    m_pinList.clear();

    Component::remove();
}

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

    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
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
    m_lastFirmDir = cleanPathAbs;
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
    if( m_deviceType >= typeMPU )
    {
        QAction* mainAction = menu->addAction( QIcon(":/subc.png"),tr("Main Mcu") );
        connect( mainAction, SIGNAL(triggered()),
                       this, SLOT(slotmain()), Qt::UniqueConnection );
    }

    if( m_deviceType == typeMCU )
    {
        QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load firmware") );
        connect( loadAction, SIGNAL(triggered()),
                       this, SLOT(slotLoad()), Qt::UniqueConnection );

        QAction* reloadAction = menu->addAction( QIcon(":/reload.png"),tr("Reload firmware") );
        connect( reloadAction, SIGNAL(triggered()),
                         this, SLOT(slotReload()), Qt::UniqueConnection );

        menu->addSeparator();

        QAction* loadDaAction = menu->addAction( QIcon(":/open.png"),tr("Load EEPROM data from file") );
        connect( loadDaAction, SIGNAL(triggered()),
                         this, SLOT(loadEEPROM()), Qt::UniqueConnection );

        QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save EEPROM data to file") );
        connect( saveDaAction, SIGNAL(triggered()),
                         this, SLOT(saveEEPROM()), Qt::UniqueConnection );
    }
    //if( m_deviceType >= typeMPU )
    {
        menu->addSeparator();

        QAction* openRamTab = menu->addAction( QIcon(":/terminal.png"),tr("Open Mcu Monitor.") );
        connect( openRamTab, SIGNAL(triggered()),
                       this, SLOT(slotOpenMcuMonitor()), Qt::UniqueConnection );
    }
    if( m_deviceType == typeMCU )
    {
        QMenu* serMonMenu = menu->addMenu( tr("Open Serial Monitor.") );

        QSignalMapper* sm = new QSignalMapper(this);
        for( uint i=0; i<m_eMcu.m_usarts.size(); ++i )
        {
            QAction* openSerMonAct = serMonMenu->addAction( "USart"+QString::number(i+1) );
            connect( openSerMonAct, SIGNAL(triggered()), sm, SLOT(map()) );
            sm->setMapping( openSerMonAct, i+1 );
        }
        connect( sm, SIGNAL(mapped(int)), this, SLOT(slotOpenTerm(int)) );
    }

    /*QAction* openSerial = menu->addAction( QIcon(":/terminal.png"),tr("Open Serial Port.") );
    connect( openSerial, SIGNAL(triggered()),
                   this, SLOT(slotOpenSerial()), Qt::UniqueConnection );*/

    menu->addSeparator();

    if( !event )
    {
        QAction* propertiesAction = menu->addAction( QIcon( ":/properties.png"),tr("Properties") );
        connect( propertiesAction, SIGNAL( triggered()),
                             this, SLOT(slotProperties()), Qt::UniqueConnection );
        menu->addSeparator();
}   }

void Mcu::slotmain()
{
    m_pSelf = this;
    m_eMcu.setMain();
    Circuit::self()->update();
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

void Mcu::setSerialMon( int s )
{
    if( s>=0 ) slotOpenTerm( s );
}

void Mcu::addPin( QString id, QString type, QString label,
                  int pos, int xpos, int ypos, int angle, int length )
{
    if( type == "nc" ) Chip::addPin( id, type, label, pos, xpos, ypos, angle, length );
    else{
        IoPin* pin = NULL;
        if( !type.isEmpty() && !type.contains("nul") )
        {
            if( type == "rst" )
                 pin = m_resetPin = new IoPin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this, input );
            else pin = m_eMcu.getIoPin( id ); // Control Port

        }
        else pin = m_eMcu.getIoPin( id ); // I/O Port

        if( !pin ) return;

        QColor color = Qt::black;
        if( !m_isLS ) color = QColor( 250, 250, 200 );

        if( type.startsWith("nul") )
        {
            pin->setVisible( false );
            pin->setLabelText( "" );
        }
        else if( type.startsWith("inv") ) pin->setInverted( true );

        pin->setPackageType( type );
        pin->setPos( QPoint( xpos, ypos ) );
        pin->setPinAngle( angle );
        pin->setLength( length );
        pin->setLabelText( label );
        pin->setLabelColor( color );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        m_pinList.append( pin );
    }
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

void Mcu::enableExtOsc( bool en )
{
    m_extClock = en;
    if( m_clkPin[0] == NULL ) return;

    for( int i=0; i<2; ++i )
        if( m_clkPin[i] ){
            m_clkPin[i]->controlPin( en, en );
            m_clkPin[i]->setUnused( en );
        }

    if( en ){
        for( int i=0; i<2; ++i )
            if( m_clkPin[i] ) m_clkPin[i]->setPinMode( input );
    }
}

bool Mcu::wdtEnabled()
{
    if( !m_eMcu.m_wdt ) return false;
    return m_eMcu.m_wdt->enabled();
}

void Mcu::enableWdt( bool en ) { if( m_eMcu.m_wdt ) m_eMcu.m_wdt->enable( en ); }

/*void Mcu::createCfgWord( QString name, uint16_t addr, uint16_t v )
{

    m_eMcu.m_cfgWords.insert( addr, v );
}*/

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

#include "moc_mcu.cpp"
