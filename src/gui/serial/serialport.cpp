/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QPainter>
#include <QMenu>

#include "serialport.h"
#include "serialmon.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "usarttx.h"
#include "usartrx.h"
#include "iopin.h"
#include "utils.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* SerialPort::construct( QObject* parent, QString type, QString id )
{ return new SerialPort( parent, type, id ); }

LibraryItem* SerialPort::libraryItem()
{
    return new LibraryItem(
        "SerialPort",
        tr("Perifericals"),
        "serialport.png",
        "SerialPort",
        SerialPort::construct );
}

SerialPort::SerialPort( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , UsartModule( NULL, id+"-Uart" )
          , eElement( (id+"-eElement") )
{
    m_area = QRect(-34, -16, 160, 32 );
    setLabelPos(-20,-32 );

    m_pin.resize(2);

    IoPin* pinTx = new IoPin( 180, QPoint(-40,-8), id+"-pin0", 0, this, output );
    pinTx->setLabelText( "Tx" );
    pinTx->setOutHighV( 5 );
    m_pin[0] = pinTx;
    m_sender->setPins( {pinTx} );

    IoPin* pinRx = new IoPin( 180, QPoint(-40, 8), id+"-pin1", 0, this, input );
    pinRx->setLabelText( "Rx" );
    m_pin[1] = pinRx;
    m_receiver->setPins( {pinRx} );

    m_serial = new QSerialPort( this );
    m_receiving = false;

    m_flowControl = QSerialPort::NoFlowControl;
    setBaudRate( 9600 );

    m_button = new QPushButton( );
    m_button->setMaximumSize( 36, 20 );
    m_button->setGeometry(-36,-20, 36, 20 );
    m_button->setCheckable( true );
    m_button->setText( "Open" );

    QFont font = m_button->font();
    font.setFamily("Ubuntu");
    font.setPixelSize(12);
    m_button->setFont( font );

    m_proxy = Circuit::self()->addWidget( m_button );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-4,-10) );

    connect( m_button, &QPushButton::clicked,
                 this, &SerialPort::onbuttonclicked, Qt::UniqueConnection);

    connect( m_serial, &QSerialPort::readyRead,
                 this, &SerialPort::readData, Qt::UniqueConnection );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { "Main", {
//new BoolProp  <Chip>( "Logic_Symbol","","", this, &Chip::logicSymbol, &Chip::setLogicSymbol ),
new StrProp<SerialPort>( "Port", tr("Port Name"),"", this, &SerialPort::port,  &SerialPort::setPort ),
    } } );

    addPropGroup( { "Config", {
new IntProp<SerialPort>("Baudrate", tr("Baudrate"),"_Bauds", this, &SerialPort::baudRate, &SerialPort::setBaudRate,0,"uint" ),
new IntProp<SerialPort>("DataBits", tr("Data Bits"),"_Bits", this, &SerialPort::dataBits, &SerialPort::setDataBits,0,"uint" ),
new IntProp<SerialPort>("StopBits", tr("Stop Bits"),"_Bits", this, &SerialPort::stopBits, &SerialPort::setStopBits,0,"uint" ),
    }, 0 } );
    addPropGroup( {"Hidden", {
new BoolProp<SerialPort>("SerialMon","","", this, &SerialPort::serialMon, &SerialPort::setSerialMon ),
}, groupHidden} );
}
SerialPort::~SerialPort(){}

void SerialPort::stamp()
{
    m_serData.clear();
    m_uartData.clear();
    m_sender->enable( true );
    m_receiver->enable( true );
    m_sending = false;
    m_receiving = false;
}

void SerialPort::updateStep()
{
    if( m_serData.size() )
    {
        if( m_serial->isOpen() ) m_serial->write( m_serData );
        m_serData.clear();
    }
    else m_receiving = false;

    if( m_uartData.size() && !m_sending ) Simulator::self()->addEvent( 1, this );

    update();
}

void SerialPort::runEvent()
{
    if( m_uartData.isEmpty() ) return;
    sendByte( m_uartData.at( 0 ) ); // Start transaction
    m_uartData = m_uartData.right( m_uartData.size()-1 );
    m_sending = true;
}

void SerialPort::open()
{
    if( m_serial->isOpen() ) close();

    m_serial->setPortName( m_portName );
    m_serial->setBaudRate( m_baudRate );
    m_serial->setDataBits( (QSerialPort::DataBits)m_dataBits );
    m_serial->setParity( (QSerialPort::Parity)m_parity );
    m_serial->setStopBits( (QSerialPort::StopBits)m_stopBits );
    m_serial->setFlowControl( QSerialPort::NoFlowControl/*m_flowControl*/ );

    if( m_serial->open( QIODevice::ReadWrite ) )
    {
        qDebug()<<"Connected to" << m_portName;
        m_button->setText( "Close" );
    }else{
        m_button->setChecked( false );
        MessageBoxNB( "Error", tr("Cannot Open Port %1:\n%2.").arg(m_portName).arg(m_serial->errorString()) );
    }
    m_receiving = false;
    update();
}

void SerialPort::close()
{
    if( m_serial->isOpen() ) m_serial->close();
    m_button->setText( tr("Open") );
    m_receiving = false;
    m_sending = false;
    update();
}

void SerialPort::readData()
{
    m_uartData += m_serial->readAll();
}

void SerialPort::byteReceived( uint8_t byte )
{
    m_receiver->getData();
    if( m_monitor ) m_monitor->printIn( byte );
    m_serData.append( byte );
    m_receiving = true;
}

void SerialPort::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
    if( m_uartData.size() )
    {
        uint8_t byte = m_uartData.at( 0 );
        m_uartData = m_uartData.right( m_uartData.size()-1 );
        sendByte( byte );
    }
    else m_sending = false;
}

void SerialPort::slotClose()
{
    close();
    Circuit::self()->removeComp( this );
}

void SerialPort::onbuttonclicked()
{
    if( m_button->isChecked() ) open();
    else                        close();
}

void SerialPort::slotOpenTerm()
{
    openMonitor( m_id, 0 );
}

void SerialPort::setSerialMon( bool s )
{
    if( s ) slotOpenTerm();
}

void SerialPort::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* openSerMon = menu->addAction( QIcon(":/terminal.svg"),tr("Open Serial Monitor.") );
    connect( openSerMon, &QAction::triggered,
                   this, &SerialPort::slotOpenTerm, Qt::UniqueConnection );

    menu->addSeparator();
    Component::contextMenu( event, menu );
}

void SerialPort::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush( Qt::darkBlue );
    p->drawRoundedRect( m_area, 4, 4 );

    if( m_serial->isOpen() )
    {
        if( m_sending ) p->setBrush( Qt::yellow );
        else            p->setBrush( Qt::red );
    }
    else p->setBrush( Qt::black );
    p->drawRoundedRect( -21,-11, 8, 6, 2, 2 ); // Tx led

    if( m_serial->isOpen() )
    {
        if( m_receiving ) p->setBrush( Qt::yellow );
        else              p->setBrush( Qt::red );
    }
    else p->setBrush( Qt::black );
    p->drawRoundedRect( -21,  5, 8, 6, 2, 2 ); // Rx led
    //p->drawEllipse( 32,-6, 12, 12);
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( QColor( 250, 210, 150 ) );
    p->setPen(pen);
    QFont font = p->font();
    font.setPixelSize(11);
    p->setFont( font );
    p->drawText( 40, 5, m_portName );
}

#include "moc_serialport.cpp"
