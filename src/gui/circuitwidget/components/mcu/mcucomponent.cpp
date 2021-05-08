/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include <qstringlist.h>
#include <QDomDocument>
#include <QFileInfo>

#include "mainwindow.h"
#include "mcucomponent.h"
#include "mcucomponentpin.h"
#include "baseprocessor.h"
#include "terminalwidget.h"
#include "componentselector.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "connector.h"
#include "simulator.h"
#include "serialport.h"
#include "serialterm.h"
#include "simuapi_apppath.h"
#include "memdata.h"
#include "mcumonitor.h"
#include "utils.h"

static const char* McuComponent_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Program"),
    QT_TRANSLATE_NOOP("App::Property","Mhz"),
    QT_TRANSLATE_NOOP("App::Property","Auto Load"),
    QT_TRANSLATE_NOOP("App::Property","Init gdb")
};

McuComponent* McuComponent::m_pSelf = 0l;

McuComponent::McuComponent( QObject* parent, QString type, QString id )
            : Chip( parent, type, id )
{
    Q_UNUSED( McuComponent_properties );

    qDebug() << "        Initializing"<<m_id<<"...";
    m_pSelf = this;
    m_attached  = false;
    m_autoLoad  = false;
    m_crashed   = false;

    m_mcuMonitor = NULL;

    m_processor  = 0l;
    m_symbolFile = "";
    m_device     = "";
    m_subcDir    = "";
    m_error      = 0;
    m_warning    = 0;
    m_cpi = 1;

    // Id Label Pos set in Chip::initChip
    m_color = QColor( 50, 50, 70 );

    QSettings* settings = MainWindow::self()->settings();
    m_lastFirmDir = settings->value("lastFirmDir").toString();

    if( m_lastFirmDir.isEmpty() )
        m_lastFirmDir = QCoreApplication::applicationDirPath();

    Simulator::self()->addToUpdateList( this );
    Circuit::self()->update();
}
McuComponent::~McuComponent()
{
    if( m_mcuMonitor ) delete m_mcuMonitor;
}

QList<propGroup_t> McuComponent::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Logic_Symbol", tr("Logic Symbol"),""} );
    mainGroup.propList.append( {"Mhz", tr("Frequency"),"MHz"} );
    mainGroup.propList.append( {"Program", tr("Fimware"),""} );
    mainGroup.propList.append( {"Auto_Load", tr("Auto Load Firmware at Start"),""} );
    return {mainGroup};
}

void McuComponent::attach()
{
    if( m_processor->getLoadStatus() )
    {
        m_processor->setFreq( m_freq/m_cpi );

        if( m_autoLoad ) load( m_symbolFile );
        emit openSerials();
    }
}

void McuComponent::initialize()
{
    reset();
}

void McuComponent::updateStep()
{
    if( m_crashed )
    {
        Simulator::self()->setWarning( m_warning );
        update();
    }
    if( m_mcuMonitor
     && m_mcuMonitor->isVisible() ) m_mcuMonitor->updateStep();
}

void McuComponent::initChip()
{
    QString compName = m_id.split("-").first(); // for example: "atmega328-1" to: "atmega328"

    QString xmlFile = ComponentSelector::self()->getXmlFile( compName );

    QDomDocument domDoc = fileToDomDoc( xmlFile, "McuComponent::initChip" );
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
            if( element.attribute("name")==compName )
            {
                // Get package file
                QDir dataDir( xmlFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                m_pkgeFile = dataDir.filePath( element.attribute( "package" ) )+".package";

                // Get data file
                QString dataFile = dataDir.filePath( element.attribute( "data" ) )+".data";
                m_processor->setDataFile( dataFile );

                // Get device
                m_device = element.attribute( "device" );
                m_processor->setDevice( m_device );
                break;
            }
            node = node.nextSibling();
        }
        rNode = rNode.nextSibling();
    }
    if( m_device != "" ) Chip::initChip();
    else
    {
        MessageBoxNB( "McuComponent::initChip", "                               \n"+
                  tr("Chip not Found: %1").arg(compName) );
        m_error = 1;
    }
}

void McuComponent::updatePin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
{
    Pin* pin = 0l;
    for( int i=0; i<m_pinList.size(); i++ )
    {
        McuComponentPin* mcuPin = m_pinList[i];
        QString pinId = mcuPin->id();

        if( id == pinId )
        {
            pin = mcuPin->pin();
            break;
        }
    }
    if( !pin )
    {
        qDebug() <<"McuComponent::updatePin Pin Not Found:"<<id<<type<<label;
        return;
    }
    pin->setLabelText( label );
    pin->setPos( QPoint(xpos, ypos) );
    pin->setPinAngle( angle );
    pin->setLabelPos();
    //m_pins.append( pin );

    type = type.toLower();

    if( type == "gnd"
     || type == "vdd"
     || type == "vcc"
     || type == "unused"
     || type == "nc" ) pin->setUnused( true );
    else               pin->setUnused( false );

    if( type == "inverted" ) pin->setInverted( true );
    else                     pin->setInverted( false );

    if( type == "null" )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
    }
    else pin->setVisible( true );

    if( m_isLS ) pin->setLabelColor( QColor( 0, 0, 0 ) );
    else         pin->setLabelColor( QColor( 250, 250, 200 ) );

    pin->isMoved();
}

void McuComponent::setFreq( double freq )
{
    if     ( freq < 0  )  freq = 0;
    else if( freq > 100 ) freq = 100;

    m_freq = freq;
}

void McuComponent::reset()
{
    m_warning = 0;
    m_crashed = false;
    m_processor->reset();
}

void McuComponent::terminate()
{
    qDebug() <<"        Terminating"<<m_id<<"...";
    m_processor->terminate();
    m_pSelf = 0l;
    qDebug() <<"     ..."<<m_id<<"Terminated\n";
}

void McuComponent::remove()
{
    emit closeSerials();

    Simulator::self()->remFromUpdateList( this );
    for( McuComponentPin* mcupin : m_pinList )
    {
        Pin* pin = mcupin->pin();
        if( pin->connector() ) pin->connector()->remove();
    }
    terminate();
    m_pinList.clear();

    Component::remove();
}

void McuComponent::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
    }
}

void McuComponent::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    event->accept();

    QAction* mainAction = menu->addAction( QIcon(":/subc.png"),tr("Main Mcu") );
    connect( mainAction, SIGNAL(triggered()),
                   this, SLOT(slotmain()), Qt::UniqueConnection );

    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load firmware") );
    connect( loadAction, SIGNAL(triggered()),
                   this, SLOT(slotLoad()), Qt::UniqueConnection );

    QAction* reloadAction = menu->addAction( QIcon(":/reload.png"),tr("Reload firmware") );
    connect( reloadAction, SIGNAL(triggered()),
                     this, SLOT(slotReload()), Qt::UniqueConnection );

    menu->addSeparator();
    QAction* loadDaAction = menu->addAction( QIcon(":/load.png"),tr("Load EEPROM data") );
    connect( loadDaAction, SIGNAL(triggered()),
                     this, SLOT(loadData()), Qt::UniqueConnection );

    QAction* saveDaAction = menu->addAction(QIcon(":/save.png"), tr("Save EEPROM data") );
    connect( saveDaAction, SIGNAL(triggered()),
                     this, SLOT(saveData()), Qt::UniqueConnection );

    menu->addSeparator();
    QAction* openRamTab = menu->addAction( QIcon(":/terminal.png"),tr("Open Mcu Monitor.") );
    connect( openRamTab, SIGNAL(triggered()),
                   this, SLOT(slotOpenMcuMonitor()), Qt::UniqueConnection );

    QAction* openTerminal = menu->addAction( QIcon(":/terminal.png"),tr("Open Serial Monitor.") );
    connect( openTerminal, SIGNAL(triggered()),
                     this, SLOT(slotOpenTerm()), Qt::UniqueConnection );

    QAction* openSerial = menu->addAction( QIcon(":/terminal.png"),tr("Open Serial Port.") );
    connect( openSerial, SIGNAL(triggered()),
                   this, SLOT(slotOpenSerial()), Qt::UniqueConnection );

    menu->addSeparator();
}

void McuComponent::slotmain()
{
    m_pSelf = this;
    m_processor->setMain();
    Circuit::self()->update();
}

void McuComponent::slotOpenMcuMonitor()
{
    if( !m_mcuMonitor )
    {
        m_mcuMonitor = new MCUMonitor( CircuitWidget::self(), m_processor );
        m_mcuMonitor->setWindowTitle( idLabel() );
    }
    m_mcuMonitor->show();
}

void McuComponent::slotOpenSerial()
{
    Component* ser = Circuit::self()->createItem( "SerialPort", "SerialPort-"+Circuit::self()->newSceneId());
    ser->setPos( pos() );
    Circuit::self()->addItem( ser );

    SerialPort* serial = static_cast<SerialPort*>(ser);
    serial->setMcuId( this->objectName() );
}

void McuComponent::slotOpenTerm()
{
    Component* ser = Circuit::self()->createItem( "SerialTerm", "SerialTerm-"+Circuit::self()->newSceneId());
    ser->setPos( pos() );
    Circuit::self()->addItem( ser );

    Component* comp = this;
    if( this->isMainComp() ) comp = m_subcircuit;

    SerialTerm* serial = static_cast<SerialTerm*>(ser);
    serial->setMcuId( comp->objectName() );
}

void McuComponent::slotLoad()
{
    const QString dir = m_lastFirmDir;
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Firmware"), dir,
                       tr("All files (*.*);;ELF Files (*.elf);;Hex Files (*.hex)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    load( fileName );
}

void McuComponent::slotReload()
{
    if( m_processor->getLoadStatus() ) load( m_symbolFile );
    else QMessageBox::warning( 0, tr("No File:"), tr("No File to reload ") );
}

void McuComponent::load( QString fileName )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();

    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( fileName );
    QString cleanPathAbs = circuitDir.cleanPath( fileNameAbs );

    if( m_processor->loadFirmware( cleanPathAbs ) )
    {
        if( !m_attached ) attachPins();

        if( !m_varList.isEmpty() ) m_processor->getRamTable()->loadVarSet( m_varList );

        m_symbolFile = circuitDir.relativeFilePath( fileName );
        m_lastFirmDir = cleanPathAbs;

        QSettings* settings = MainWindow::self()->settings();
        settings->setValue( "lastFirmDir", m_symbolFile );
    }
    //else QMessageBox::warning( 0, tr("Error:"), tr("Could not load: \n")+ fileName );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void McuComponent::setSubcDir( QString dir ) // Used in subcircuits
{
    m_subcDir = dir;
    setProgram( m_symbolFile );
}

QStringList McuComponent::varList()
{
    if( m_processor->getLoadStatus() )
        return m_processor->getRamTable()->getVarSet();

    return m_varList;
}

void McuComponent::setProgram( QString pro )
{
    if( pro == "" ) return;
    m_symbolFile = pro;

    QDir circuitDir;
    if( m_subcDir != "" ) circuitDir.setPath( m_subcDir );
    else circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( m_symbolFile );

    if( QFileInfo::exists( fileNameAbs ) ) load( m_symbolFile );// Load firmware at circuit load
}

void McuComponent::setEeprom( QVector<int> eep )
{
    if( eep.size() > 1 ) m_processor->setEeprom( &eep );
}

QVector<int> McuComponent::eeprom()
{
    return *(m_processor->eeprom());
}

void McuComponent::loadData()
{
    bool resize = false;
   //if( !m_processor->getLoadStatus() ) resize = true; // No eeprom initialized yet

   QVector<int>* eeprom = m_processor->eeprom();
   MemData::loadData( eeprom, resize );
   m_processor->setEeprom( eeprom );
   if( m_mcuMonitor ) m_mcuMonitor->tabChanged( 2 );
}

void McuComponent::saveData()
{
    MemData::saveData( m_processor->eeprom() );
}

void McuComponent::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );

    if( m_crashed )
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
        p->drawRoundedRect( m_area.width()/2-2, m_area.height()/2-2, 4, 4 , 2, 2);
    }
}

#include "moc_mcucomponent.cpp"
