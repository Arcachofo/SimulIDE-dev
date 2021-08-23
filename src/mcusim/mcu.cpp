/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include <QDomDocument>
#include <QFileInfo>
#include <QTranslator>
#include <QSignalMapper>

#include "mcu.h"
#include "mcuport.h"
#include "mcupin.h"
#include "mcucreator.h"

#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "componentselector.h"
#include "mcumonitor.h"
#include "serialmon.h"
#include "mcuuart.h"
#include "simuapi_apppath.h"
#include "utils.h"


LibraryItem* Mcu::libraryItem()
{
    return new LibraryItem(
        "NEW_MCU",
        tr(""),
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
        //Circuit::self()->removeComp( mcu );
        mcu = NULL;
        m_error = 0;
        //m_pSelf = 0l;
    }
    return mcu;
}

Mcu::Mcu( QObject* parent, QString type, QString id )
   : McuBase( parent, type, id )
   , m_eMcu( id )
{
    m_pSelf = this;
    m_proc = &m_eMcu;
    m_device = m_id.split("-").first(); // for example: "atmega328-1" to: "atmega328"
    m_name = m_device;

    m_mcuMonitor = NULL;
    m_autoLoad  = false;
    m_serMonMask = 0;

    m_icColor = QColor( 20, 30, 60 );

    QString xmlFile = ComponentSelector::self()->getXmlFile( m_device );
    QFile file( xmlFile );

    if(( xmlFile == "" ) || ( !file.exists() ))
    {
        MessageBoxNB( "Mcu::Mcu", "                               \n"+
                  tr("xml file not found: %1").arg(xmlFile) );
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
    /*if( m_device == "" ) //return;//Chip::initChip();
    {
        m_error = 1;
        qDebug() << m_device << "ERROR!! Mcu::Mcu Chip not Found: " << m_device;
        return;
    }*/

    QSettings* settings = MainWindow::self()->settings();
    m_lastFirmDir = settings->value("lastFirmDir").toString();

    QDir dir( m_lastFirmDir );
    if( !dir.exists() ) m_lastFirmDir = Circuit::self()->getFileName();

    m_subcDir = "";

    Simulator::self()->addToUpdateList( this );
}
Mcu::~Mcu()
{
    if( m_mcuMonitor ) delete m_mcuMonitor;
}

QList<propGroup_t> Mcu::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    //mainGroup.propList.append( {"Logic_Symbol", tr("Logic Symbol"),""} );
    mainGroup.propList.append( {"Mhz", tr("Frequency"),"MHz"} );
    mainGroup.propList.append( {"Program", tr("Fimware"),""} );
    mainGroup.propList.append( {"Auto_Load", tr("Auto Load Firmware at Start"),""} );
    return {mainGroup};
}

QString Mcu::program() { return m_eMcu.getFileName(); }

void Mcu::setProgram( QString pro )
{
    if( pro == "" ) return;
    m_eMcu.m_firmware = pro;

    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( m_eMcu.m_firmware );

    if( QFileInfo::exists( fileNameAbs ) )
    { load( m_eMcu.m_firmware ); }
}

QStringList Mcu::varList()
{
    return m_eMcu.getRamTable()->getVarSet();
}

void Mcu::setVarList( QStringList vl )
{
    m_eMcu.getRamTable()->loadVarSet( vl );
}

void Mcu::setEeprom( QVector<int> eep )
{
    if( eep.size() > 1 ) m_eMcu.setEeprom( &eep );
}

QVector<int> Mcu::eeprom()
{
    return *(m_eMcu.eeprom());
}

void Mcu::initialize()
{
    m_crashed = false;
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

    if( Circuit::self()->animate( ) ) for( Pin* pin : m_pinList ) pin->updateStep();
}

void Mcu::attach()
{
    if( m_autoLoad )
    {
        if( !m_eMcu.m_firmware.isEmpty() ) load( m_eMcu.m_firmware );
}   }

void Mcu::remove()
{
    for( SerialMonitor* ser: m_serialMons ) ser->close();

    for( Pin* pin : m_pinList ) pin->removeConnector();
    m_pinList.clear();

    Component::remove();
}

void Mcu::reset()
{
    m_eMcu.initialize();
}

void Mcu::slotLoad()
{
    QDir dir( m_lastFirmDir );
    if( !dir.exists() ) m_lastFirmDir = Circuit::self()->getFileName();

    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Firmware"), m_lastFirmDir,
                       tr("All files (*.*);;ELF Files (*.elf);;Hex Files (*.hex)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    load( fileName );
}

void Mcu::slotReload()
{
    if( !m_eMcu.m_firmware.isEmpty() ) load( m_eMcu.m_firmware );
    else QMessageBox::warning( 0, tr("No File:"), tr("No File to reload ") );
}

bool Mcu::load( QString fileName )
{
    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString fileNameAbs  = circuitDir.absoluteFilePath( fileName );
    QString cleanPathAbs = circuitDir.cleanPath( fileNameAbs );

    if( Simulator::self()->simState() > SIM_STARTING )  CircuitWidget::self()->powerCircOff();

    bool ok = false;

    QString msg = loadHex( cleanPathAbs, m_eMcu.m_wordSize );
    if( msg.isEmpty() )
    {
        ok = true;
        msg ="hex file succesfully loaded";

        m_eMcu.m_firmware = circuitDir.relativeFilePath( fileName );
        m_lastFirmDir = cleanPathAbs;

        QSettings* settings = MainWindow::self()->settings();
        settings->setValue( "lastFirmDir", circuitDir.relativeFilePath( fileName ) );
    }
    qDebug() << msg << "\n";
    return ok;
}

void Mcu::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

void Mcu::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* mainAction = menu->addAction( QIcon(":/subc.png"),tr("Main Mcu") );
    connect( mainAction, SIGNAL(triggered()),
                   this, SLOT(slotmain()), Qt::UniqueConnection );

    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load firmware") );
    connect( loadAction, SIGNAL(triggered()),
                   this, SLOT(slotLoad()), Qt::UniqueConnection );

    QAction* reloadAction = menu->addAction( QIcon(":/reload.png"),tr("Reload firmware") );
    connect( reloadAction, SIGNAL(triggered()),
                     this, SLOT(slotReload()), Qt::UniqueConnection );

    QMenu* serMonMenu = menu->addMenu( tr("Open Serial Monitor.") );

    QSignalMapper* sm = new QSignalMapper(this);

    for( uint i=0; i<m_eMcu.m_usarts.size(); ++i )
    {
        QAction* openSerMonAct = serMonMenu->addAction( "USart"+QString::number(i+1) );
        openSerMonAct->setCheckable( true );

        if( m_serMonMask & (1<<i) ) openSerMonAct->setChecked( true );
        else                        openSerMonAct->setChecked( false );
        connect( openSerMonAct, SIGNAL(triggered()), sm, SLOT(map()) );
        sm->setMapping( openSerMonAct, i+1 );
    }
    connect( sm, SIGNAL(mapped(int)), this, SLOT(slotOpenTerm(int)) );

    /*QAction* loadDaAction = menu->addAction( QIcon(":/load.png"),tr("Load EEPROM data") );
    connect( loadDaAction, SIGNAL(triggered()),
                     this, SLOT(loadData()), Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save EEPROM data") );
    connect( saveDaAction, SIGNAL(triggered()),
                     this, SLOT(saveData()), Qt::UniqueConnection );*/

    /*QAction* openSerial = menu->addAction( QIcon(":/terminal.png"),tr("Open Serial Port.") );
    connect( openSerial, SIGNAL(triggered()),
                   this, SLOT(slotOpenSerial()), Qt::UniqueConnection );*/


    QAction* openRamTab = menu->addAction( QIcon(":/terminal.png"),tr("Open Mcu Monitor.") );
    connect( openRamTab, SIGNAL(triggered()),
                   this, SLOT(slotOpenMcuMonitor()), Qt::UniqueConnection );

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
    /// m_proc->setMain();
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
    if( m_serMonMask & (1<<(num-1)) )
    {
        for( SerialMonitor* ser : m_serialMons )
        {
            if( ser->uartNum() == num )
            { ser->show(); break; }
    }   }
    else{
        m_serMonMask |= 1<<(num-1);

        SerialMonitor* ser = new SerialMonitor( CircuitWidget::self(), &m_eMcu, num );
        ser->setWindowTitle( this->idLabel()+" - Uart"+QString::number(num) );
        ser->show();
        m_eMcu.m_usarts.at(num-1)->setMonitor( ser );
        m_serialMons.append( ser );
}   }

void Mcu::addPin( QString id, QString type, QString label,
                     int pos, int xpos, int ypos, int angle, int length )
{
    McuPin* pin = NULL;

    if( type.contains("IO") )
    {
        bool portPin = false;
        int pinNum = id.right(1).toInt( &portPin );
        if( portPin )
        {
            QString portName = "PORT"+id.mid(1,1);
            McuPort* port = m_eMcu.m_ports.getPort( portName );
            if( port )
            {
                pin = port->getPin( pinNum );
                if( pin )
                {
                    if( type.contains("null") ) { pin->setVisible( false ); pin->setLabelText( "" ); }
                    pin->setPos( QPoint( xpos, ypos ) );
                    pin->setPinAngle( angle );
                    pin->setLength( length );
                    pin->setLabelText( label );
                    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
                    m_pinList.append( pin );
    }   }   }   }
    if( !pin ) Chip::addPin( id, type, label, pos, xpos, ypos, angle, length );
}

QString Mcu::loadHex( QString file, int WordSize )
{
    qDebug() << m_device << " Loading hex file: \n" << file;

    QStringList lineList = fileToStringList( file, "Mcu::loadHex" );

    int nLine = 0;
    int addr;
    int nBytes;
    int type;
    int checksum;
    int hiByte;
    uint16_t data;
    bool ok;

    for( QString line : lineList )
    {
        checksum = 0;
        line = line.remove( " " );
        if( line.isEmpty() ) continue;

        if( !(line.left(1) == ":") ) return "Error: Wrong Start code at line "+QString::number(nLine);
        line = line.remove( 0, 1 );

        nBytes = line.left( 2 ).toInt( &ok, 16 );
        int lineSize = 2+4+2+nBytes*2+2;
        if( line.size() != lineSize ) return "Error: Wrong line size at line "+QString::number(nLine);
        checksum += nBytes;

        addr = line.mid( 2, 4 ).toInt( &ok, 16 );
        addr /= WordSize;
        checksum += line.mid( 2, 2 ).toInt( &ok, 16 );
        checksum += line.mid( 4, 2 ).toInt( &ok, 16 );

        type = line.mid( 6, 2 ).toInt( &ok, 16 );
        if     ( type == 1 ) return ""; // Reached End Of File
        else if( type != 0 ) continue;  //return "Error: Not valid Record type at line "+QString::number(nLine);
        checksum += type;

        int i;
        for( i=8; i<8+nBytes*2; i+=2*WordSize )
        {
            data = line.mid( i, 2 ).toInt( &ok, 16 );
            checksum += data;

            if( WordSize == 2 )
            {
                hiByte = line.mid( i+2, 2 ).toInt( &ok, 16 );
                data += (hiByte<<8);
                checksum += hiByte;
            }
            m_eMcu.m_progMem[addr] = data;
            addr++;
        }
        checksum += line.mid( i, 2 ).toInt( &ok, 16 );
        if( checksum & 0xFF ) return "Error: CheckSum Error at line "+QString::number(nLine);
        nLine++;
    }
    return "Error: No End Of File reached";
}

void Mcu::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );

    if( m_crashed ) /// TODO
    {
static double opCount = 0;
        opCount += 0.04;
        if( opCount > 0.6 ) opCount = 0;
        p->setOpacity( opCount );
        p->fillRect( boundingRect(), Qt::yellow  );
    }

    if( m_pSelf == this )
    {
        QPen pen( Qt::black, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p->setPen( pen );
        p->setBrush( Qt::yellow );
        if( m_width == m_height )
            p->drawRoundedRect( 4, 4, 4, 4 , 2, 2);
        else
            p->drawRoundedRect( m_area.width()/2-2, -1, 4, 4 , 2, 2);
    }
}

#include "moc_mcu.cpp"
