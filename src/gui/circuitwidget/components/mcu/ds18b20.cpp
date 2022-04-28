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

#include <QDebug>

#include <QPainter>
#include <QPushButton>
#include <QGraphicsProxyWidget>

#include "ds18b20.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"

#include "doubleprop.h"

Component* Ds18b20::construct( QObject* parent, QString type, QString id )
{ return new Ds18b20( parent, type, id ); }

LibraryItem* Ds18b20::libraryItem()
{
    return new LibraryItem(
        "DS18B20",
        tr( "Sensors" ),
        "ic_comp.png",
        "DS18B20",
        Ds18b20::construct );
}

Ds18b20::Ds18b20( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( id )
{
    m_area = QRect(-28,-16, 56, 32 );

    m_temp = 22;
    m_tempInc = 0.5;

    m_pin.resize(1);
    m_pin[0] = m_inpin = new IoPin( 180, QPoint(-36, 0), id+"-inPin", 0, this, openCo );
    m_inpin->setOutHighV( 5 );
    m_inpin->setLabelColor( QColor( 250, 250, 200 ) );
    m_inpin->setLabelText("DQ");

    QPushButton* u_button = new QPushButton();
    u_button->setMaximumSize( 9, 9 );
    u_button->setGeometry(-5,-5, 9, 9);
    u_button->setCheckable( false );
    u_button->setIcon(QIcon(":/su.png"));

    QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( u_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( -7, 4 ) );

    QPushButton* d_button = new QPushButton();
    d_button->setMaximumSize( 9, 9 );
    d_button->setGeometry(-5,-5, 9, 9);
    d_button->setCheckable( false );
    d_button->setIcon(QIcon(":/giu.png"));

    proxy = Circuit::self()->addWidget( d_button );
    proxy->setParentItem( this );
    proxy->setPos( QPoint( 9, 4 ) );

    connect( u_button, SIGNAL( pressed()),
             this,     SLOT  ( upbuttonclicked()) );

    connect( d_button, SIGNAL( pressed()),
             this,     SLOT  ( downbuttonclicked()) );

    m_font.setFamily("Ubuntu");
    m_font.setBold( true );
    setLabelPos(-24,-28 );

    addPropGroup( { tr("Main"), {
new DoubProp<Ds18b20>( "Temp"   , tr("Temperature")    ,"°C", this, &Ds18b20::temp   , &Ds18b20::setTemp ),
new DoubProp<Ds18b20>( "TempInc", tr("Temp. increment"),"°C", this, &Ds18b20::tempInc, &Ds18b20::setTempInc ),
    }} );
}
Ds18b20::~Ds18b20(){}

void Ds18b20::stamp()   // Called at Simulation Start
{
    m_lastTime = 0;
    m_reset = false;
    m_write = false;
    m_lastIn = false;
    m_inpin->setPinMode( openCo );
    m_inpin->setOutState( true );
    m_inpin->changeCallBack( this, true );
}

void Ds18b20::voltChanged()                              // Called when Input Pin changes
{
    bool inState = m_inpin->getVolt() > 2.5;
    if( m_lastIn && !inState ) m_lastTime = Simulator::self()->circTime(); // Falling edge
    else if( !m_lastIn && inState )                                        // Rising edge
    {
        uint64_t time = Simulator::self()->circTime()-m_lastTime; // in picoseconds

        if( time > 480*1e6 )                              // > 480 us : Reset
        {
            reset();
        }
        else if( m_reset )
        {
            if( time > 60*1e6 )                         // > 60 us : 0 received
            {
                if( m_write ) {;} // ERROR Master is reading data, Read pulse should be < 15 us
                else          readBit( 0 );
            }
            else if( time < 15*1e6 )                         // < 15 us : 1 received Or Read pulse
            {
                if( m_write ) writeBit();
                else          readBit( 1 );
            }
        }
    }
    m_lastIn = inState;
}

void Ds18b20::runEvent()
{
    if( m_pullDown )   // Pull down
    {
        m_pullDown = false;
        m_inpin->sheduleState( false, 0 );
        Simulator::self()->addEvent( m_pulse, this );
    }
    else              // Release
    {
        m_inpin->sheduleState( true, 0 );
        if( !m_reset ) m_reset = true;
        m_inpin->changeCallBack( this, true ); // Receive voltChange() CallBacks again
    }
}

void Ds18b20::byteReceived() // a byte has been received (it's in m_rxReg)
{
    qDebug() <<"Ds18b20::byteReceived"<< m_rxReg; // Print received byte in bottom panel
    // Do whatever
}

void Ds18b20::byteSent() // Last byte has been sent
{
    // Do whatever
}

void Ds18b20::sendByte( uint8_t data )
{
    m_txReg = data;
    m_bit = 1;
    m_write = true;
    m_pulse = 15*1e6;  // Keep line low for 15 us
    //m_inpin->changeCallBack( this, false ); // Stop receiving voltChange() CallBacks
}

void Ds18b20::writeBit()
{
    if( (m_txReg & m_bit) == 0 )
    {
        Simulator::self()->addEvent( 1*1e6, this ); // Pull down in 1 us
        m_pullDown = true;
        m_inpin->changeCallBack( this, false ); // Stop receiving voltChange() CallBacks
    }
    if( m_bit == 1<<7 ) // Byte sent
    {
        m_write = false;
        byteSent();
    }
    else m_bit <<= 1;
}

void Ds18b20::readBit( uint8_t bit )
{
    if( bit ) m_rxReg |= m_bit;

    if( m_bit == 1<<7 )          // Complete byte received
    {
        m_bit = 1;
        byteReceived();
        m_rxReg = 0;
    }
    else m_bit <<= 1;
}

void Ds18b20::reset()
{
    m_rxReg = 0;
    m_bit = 1;
    m_reset = false;
    m_write = false;
    m_pullDown = true;
    m_pulse = 80*1e6;                            // Keep line low for 80 us (60 to 240 us)
    m_inpin->changeCallBack( this, false );      // Stop receiving voltChange() CallBacks
    Simulator::self()->addEvent( 30*1e6, this ); // Send ack after 30 us (15 to 60 us)
}

void Ds18b20::upbuttonclicked()
{
    m_temp += m_tempInc;
    if( m_temp > 125 )  m_temp = 125;
    m_changed = true;
    update();
}

void Ds18b20::downbuttonclicked()
{
    m_temp -= m_tempInc;
    if( m_temp < -55 ) m_temp = -55;
    m_changed = true;
    update();
}

void Ds18b20::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );

    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( QRect(-11,-14, 33, 15 ),2,2 );

    m_font.setPixelSize(9);
    p->setFont( m_font );
    p->setPen( QColor(0, 0, 0) );
    p->drawText( -9, -3, "C° "+QString::number( m_temp ) );
}

//#include "moc_ds18b20.cpp"
