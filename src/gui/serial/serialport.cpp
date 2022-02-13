/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QPainter>

#include "serialport.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "usarttx.h"
#include "usartrx.h"
#include "iopin.h"
#include "utils.h"

static const char* SerialPort_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Steps"),
    QT_TRANSLATE_NOOP("App::Property","Port Name"),
    QT_TRANSLATE_NOOP("App::Property","BaudRate"),
    QT_TRANSLATE_NOOP("App::Property","DataBits"),
    QT_TRANSLATE_NOOP("App::Property","Parity"),
    QT_TRANSLATE_NOOP("App::Property","StopBits"),
    QT_TRANSLATE_NOOP("App::Property","FlowControl")
};

Component* SerialPort::construct( QObject* parent, QString type, QString id )
{ return new SerialPort( parent, type, id ); }

LibraryItem* SerialPort::libraryItem()
{
    return new LibraryItem(
        "SerialPort",
        tr("Perifericals"),
        "SerialPort.png",
        "SerialPort",
        SerialPort::construct );
}

SerialPort::SerialPort( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , UsartModule( NULL, id+"-Uart" )
          , eElement( (id+"-eElement") )
{
    Q_UNUSED( SerialPort_properties );

    m_area = QRect( -34, -16, 160, 32 );
    setLabelPos(-34,-20 );

    m_pin.resize(2);

    IoPin* pinTx = new IoPin( 180, QPoint(-40,-8), id+"-pin0", 0, this, output );
    pinTx->setLabelText( "Tx" );
    m_pin[0] = pinTx;
    m_sender->setPins( {pinTx} );

    IoPin* pinRx = new IoPin( 180, QPoint(-40, 8), id+"-pin1", 0, this, input );
    pinRx->setLabelText( "Rx" );
    m_pin[1] = pinRx;
    m_receiver->setPins( {pinRx} );

    m_serial = new QSerialPort( this );
    m_active = false;

    m_BaudRate    = QSerialPort::Baud9600;
    m_dataBits    = QSerialPort::Data8;
    m_parity      = QSerialPort::NoParity;
    m_stopBits    = QSerialPort::OneStop;
    m_flowControl = QSerialPort::NoFlowControl;

    m_button = new QPushButton( );
    m_button->setMaximumSize( 36, 20 );
    m_button->setGeometry(-36,-20, 36, 20 );
    m_button->setCheckable( true );
    m_button->setText( "Open" );

    m_proxy = Circuit::self()->addWidget( m_button );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-18,-10) );

    connect( m_button, SIGNAL( clicked() ),
                 this, SLOT(   onbuttonclicked() ), Qt::UniqueConnection);

    connect( m_serial, SIGNAL( readyRead()),
                 this, SLOT(   readData()), Qt::UniqueConnection );

    Simulator::self()->addToUpdateList( this );

    initialize();
}
SerialPort::~SerialPort(){}

void SerialPort::initialize()
{
}

void SerialPort::open()
{
    if( m_serial->isOpen() ) close();

    m_serial->setPortName( m_portName );
    m_serial->setBaudRate( m_BaudRate );
    m_serial->setDataBits( m_dataBits );
    m_serial->setParity(   m_parity );
    m_serial->setStopBits( m_stopBits );
    m_serial->setFlowControl( m_flowControl );

    if( m_serial->open( QIODevice::ReadWrite ) )
    {
        qDebug()<<"Connected to" << m_portName;
        m_button->setText( "Close" );
    }else{
        m_button->setChecked( false );
        MessageBoxNB( "Error", tr("Cannot Open Port %1:\n%2.").arg(m_portName).arg(m_serial->errorString()) );
    }
    m_active = false;
    update();
}

void SerialPort::close()
{
    if( m_serial->isOpen() ) m_serial->close();
    m_button->setText( "Open" );
    m_active = false;
    update();
}

void SerialPort::readData()
{
    m_uartData = m_serial->readAll();
    m_active = !m_active;
    m_dataIndex = 0;
    frameSent( 0 );
    update();

    /// for( int i=0; i<data.size(); i++ ) m_processor->uartIn( m_uart, data.at(i) );
}

void SerialPort::byteReceived( uint8_t byte )
{
    if( m_serial->isOpen() )
    {
        QByteArray ba;
        ba.resize(1);
        ba[0] = byte;
        m_serial->write( ba );
        m_active = !m_active;
        update();
}   }

void SerialPort::frameSent( uint8_t )
{
    if( m_dataIndex < m_uartData.size() )
    {
        sendByte( m_uartData.at( m_dataIndex ) );
        m_dataIndex++;
    }
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

void SerialPort::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush( Qt::darkBlue );
    p->drawRoundedRect( m_area, 4, 4 );

    if( m_serial->isOpen() )
    {
        if( m_active ) p->setBrush( Qt::yellow );
        else           p->setBrush( Qt::red );
        m_active = false;
    }
    else p->setBrush( Qt::black );
    p->drawEllipse( 22,-6, 12, 12);
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( QColor( 250, 210, 150 ) );
    p->setPen(pen);
    p->drawText( 40, 5, "PORT1" );
}

#include "moc_serialport.cpp"
