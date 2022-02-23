/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include <QPainter>
#include <QtNetwork>
#include <QTcpSocket>
#include <QMenu>

#include "esp01.h"
#include "itemlibrary.h"
#include "serialmon.h"
#include "simulator.h"
#include "usarttx.h"
#include "usartrx.h"
#include "iopin.h"

#include "intprop.h"

Component* Esp01::construct( QObject* parent, QString type, QString id )
{ return new Esp01( parent, type, id ); }

LibraryItem* Esp01::libraryItem()
{
    return new LibraryItem(
        "Esp01 (TCP)",
        tr("Perifericals"),
        "esp01_ico.png",
        "Esp01",
        Esp01::construct );
}

Esp01::Esp01( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , UsartModule( NULL, id+"-Uart" )
     , eElement( id )
{
    m_area = QRect(-28,-20, 56, 40 );
    m_background = ":/esp01.png";

    m_OK = "\r\nOK\r\n";
    m_ERROR = "\r\n+CWJAP:<5>\r\nERROR\r\n"; // Default reply: Error

    m_pin.resize(2);

    IoPin* pinTx = new IoPin( 180, QPoint(-36,-8), id+"-pin0", 0, this, output );
    pinTx->setLabelText( "Tx" );
    pinTx->setOutHighV( 5 );
    m_pin[0] = pinTx;
    m_sender->setPins( {pinTx} );

    IoPin* pinRx = new IoPin( 180, QPoint(-36, 8), id+"-pin1", 0, this, input );
    pinRx->setLabelText( "Rx" );
    m_pin[1] = pinRx;
    m_receiver->setPins( {pinRx} );

    setBaudRate( 115200 );

    m_tcpSocket = new QTcpSocket(this);

    Simulator::self()->addToUpdateList( this );

    connect( m_tcpSocket, SIGNAL( connected() )         ,this, SLOT( tcpConnected() ));
    connect( m_tcpSocket, SIGNAL( disconnected() )      ,this, SLOT( tcpDisconnected() ));
    connect( m_tcpSocket, SIGNAL( bytesWritten(qint64) ),this, SLOT( tcpBytesWritten(qint64) ));
    connect( m_tcpSocket, SIGNAL( readyRead() )         ,this, SLOT( tcpReadyRead() ));

    addPropGroup( { "Main", {
new IntProp<Esp01>("Baudrate", tr("Baudrate"),"_Bauds", this, &Esp01::baudRate, &Esp01::setBaudRate, "uint" ),
//new IntProp<Esp01>("DataBits", tr("Data Bits"),"_Bits", this, &Esp01::dataBits, &Esp01::setDataBits, "uint" ),
//new IntProp<Esp01>("StopBits", tr("Stop Bits"),"_Bits", this, &Esp01::stopBits, &Esp01::setStopBits, "uint" ),
        } } );
}
Esp01::~Esp01(){}

void Esp01::stamp()
{
    m_conWIFI = false;
    m_conTCP = false;
    m_dataLenght = 0;
    m_action = espNone;
    m_buffer.clear();
    m_sender->enable( true );
    m_receiver->enable( true );
}

void Esp01::updateStep()
{
    switch( m_action ) {
    case espNone:
        break;
    case tcpConnect:
        //qDebug() << "m_tcpSocket->connectToHost( m_host, m_port )"<<m_host<< m_port;
        m_tcpSocket->connectToHost( m_host, m_port );
        break;
    case tcpSend:
        //qDebug() << "m_tcpSocket->write( m_tcpData )"<<m_tcpData;
        if( m_conTCP ) m_tcpSocket->write( m_tcpData );
        break;
    case tcpClose:
        //qDebug() << "m_tcpSocket->disconnectFromHost()";
        if( m_conTCP ) m_tcpSocket->disconnectFromHost();
        break;
    }
    m_action = espNone;
}

void Esp01::runEvent()
{
    if( m_uartReply.isEmpty() ) return;
    qDebug() << "Reply:"<< m_uartReply;
    sendByte( m_uartReply.at( 0 ) ); // Start transaction
    m_uartReply = m_uartReply.right( m_uartReply.size()-1 );
}

void Esp01::byteReceived( uint8_t byte )
{
    m_receiver->getData();
    if( m_monitor ) m_monitor->printIn( byte );
    if( m_dataLenght )
    {
        if( m_tcpData.size() <= m_dataLenght-2 ) m_tcpData.append( byte );
        else if( byte == 10 ) // received \r\n
        {
            if( m_conTCP ) m_action = tcpSend;
            m_dataLenght = 0;
    }   }
    else{
        m_buffer.append( byte ); //qDebug() << m_buffer;
        if( m_buffer.right(2)  == "\r\n")
        { proccessInput(); runEvent(); }
    }
}

void Esp01::proccessInput()
{
    QString command = m_buffer.remove("\r\n");
    m_buffer.clear();
    m_uartReply = "";
qDebug() << "Command:"<< command;
    if( command.isEmpty() )
        return;

    m_link = 0;
    m_dataLenght = 0;
    m_uartReply = m_ERROR; // Default reply: Error

    if( command == "AT+RST" )
    {
        /// Reset
        m_uartReply = m_OK;
    }
    else if( command == "AT+CWMODE?" )
    {
        QByteArray mode;
        mode.setNum( m_mode );
        m_uartReply = "+CWMODE:<"+mode+">"+m_OK;
    }
    else if( command.startsWith("AT+CWMODE=") ) //
    {
        m_mode = command.right(1).toInt();
        if( m_mode == 1 || m_mode == 0 ) m_uartReply = m_OK;
        else m_mode = 0;
    }
    else if( command.startsWith("AT+CWJAP=")
          || command.startsWith("AT+CWJEAP=") ) // Connect to an AP
    {
        /// Save parameters for Query
        m_conWIFI = true;
        m_uartReply = "\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n"+m_OK;
    }
    else if( command == "AT+CWQAP" )
    {
        if( m_conTCP ) m_action = tcpClose;
        m_conWIFI = false;
        m_uartReply = m_OK;
    }
    else if( command == "AT+CIPMUX?" )
    {
        QByteArray multi;
        multi.setNum( m_multCon );
        m_uartReply = "+CIPMUX:<"+multi+">"+m_OK;
    }
    else if( command.startsWith("AT+CIPMUX=") ) // Enable/disable Multiple Connections
    {
        m_multCon = command.right(1).toInt();
        if( (m_multCon == 1) || (m_multCon == 0) ) m_uartReply = m_OK;
        else m_multCon = 0;
    }
    else if( command.startsWith("AT+CIPSTART=") ) // Start TCP Connection
    {
        if( !m_conWIFI ) return; // WIFI is not connected

        command = command.remove( 0, 12 ).remove("\"");
        QStringList param = command.split(",");
        if( m_multCon )
        {
            if( param.isEmpty() ) return;
            m_link = param.takeFirst().toInt();
        }
        if( param.isEmpty() ) return;
        QString type = param.takeFirst();
        if( type == "TCP" )
        {
            if( param.isEmpty() ) return;
            m_host = param.takeFirst();
            if( param.isEmpty() ) return;
            m_port = param.takeFirst().toInt();
            if( !m_conTCP ) m_action = tcpConnect;
            m_uartReply = "";
        }
    }
    else if( command.startsWith("AT+CIPSEND=") ) // Send data
    {
        command = command.remove( 0, 11 );
        QStringList param = command.split(",");
        if( m_multCon )
        {
            if( param.isEmpty() ) return;
            m_link = param.takeFirst().toInt();
        }
        if( param.isEmpty() ) return;
        m_dataLenght = param.takeFirst().toInt()+2; // +2 due to last \r\n
        m_tcpData.clear();
        if( m_conTCP ) m_uartReply = m_OK+"\r\n>\r\n";
    }
    else if( command.startsWith("AT+CIPCLOSE") ) // Stop TCP Connection
    {
        if( !m_conWIFI ) return; // WIFI is not connected

        if( command.contains("=") )
        {
            command = command.remove( 0, 12 );
            m_link = command.toInt();
        }
        m_action = tcpClose;
        m_uartReply ="";
    }

/// ---- TODO ----------------------------------------------

    else if( command == "AT+CMD?" )
    {
        /// Command list
    }
    else if( command == "AT+UART_CUR?" ) // Query
    {
        ///+UART_CUR:<baudrate>,<databits>,<stopbits>,<parity>,<flow control>
        ///OK
    }
    else if( command.startsWith("AT+UART_CUR") ) //  Set
    {
        ///+UART_CUR=<baudrate>,<databits>,<stopbits>,<parity>,<flow control>
        ///OK
  /*<databits>: data bits
        5: 5-bit data
        6: 6-bit data
        7: 7-bit data
        8: 8-bit data
    <stopbits>: stop bits
        1: 1-bit stop bit
        2: 1.5-bit stop bit
        3: 2-bit stop bit
    <parity>: parity bit
        0: None
        1: Odd
        2: Even
    <flow control>: flow control
        0: flow control is not enabled
        1: enable RTS
        2: enable CTS
        3: enable both RTS and CTS*/
    }
    else if( command == "AT+UART_DEF?" ) // Query
    {
        // Same but saved in flash (properties)
        ///OK
    }
    else if( command.startsWith("AT+UART_DEF") ) //  Set
    {
        // Same but saving to flash (properties)
        ///OK
    }
}

void Esp01::frameSent( uint8_t data )
{
    if( m_monitor ) m_monitor->printOut( data );
    if( m_uartReply.size() )
    {
        uint8_t byte = m_uartReply.at( 0 );
        m_uartReply = m_uartReply.right( m_uartReply.size()-1 );
        sendByte( byte );
    }
}

void Esp01::tcpConnected()
{
    connectReply( "CONNECT" );
    m_conTCP = true;
}
void Esp01::tcpDisconnected()
{
    connectReply( "CLOSED" );
    m_conTCP = false;
}
void Esp01::tcpBytesWritten( qint64 )
{
    m_uartReply = "\r\nSEND OK\r\n";
    Simulator::self()->addEvent( 1, this ); // Send Reply
}
void Esp01::tcpReadyRead()
{
    m_uartReply = m_tcpSocket->readAll();
    Simulator::self()->addEvent( 1, this ); // Send Reply
}

void Esp01::connectReply( QByteArray OP )
{
    QByteArray link;
    link.setNum( m_link );
    m_uartReply = "\r\n";
    if( m_multCon ) m_uartReply += "<"+link+">,";
    m_uartReply += OP+"\r\n"+m_OK;
    Simulator::self()->addEvent( 1, this ); // Send Reply
}

void Esp01::slotOpenTerm()
{
    openMonitor( m_id, 0 );
}

void Esp01::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* openSerMon = menu->addAction( QIcon(":/terminal.png"),tr("Open Serial Monitor.") );
    connect( openSerMon, SIGNAL(triggered()),
                   this, SLOT(slotOpenTerm()), Qt::UniqueConnection );

    menu->addSeparator();
    Component::contextMenu( event, menu );
}

void Esp01::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area, 2, 2 );
    p->drawPixmap( QRect(-28,-20, 56, 40 ), QPixmap( m_background ) );
}

#include "moc_esp01.cpp"
