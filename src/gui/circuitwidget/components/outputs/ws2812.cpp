/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "ws2812.h"
#include "itemlibrary.h"
//#include "mcucomponent.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"


Component* WS2812::construct( QObject* parent, QString type, QString id )
{
        return new WS2812( parent, type, id );
}

LibraryItem* WS2812::libraryItem()
{
    return new LibraryItem(
        tr( "WS2812 Led" ),
        tr( "Leds" ),
        "ws2812.png",
        "WS2812",
        WS2812::construct );
}

WS2812::WS2812( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( id )
      , eClockedDevice()
{
    m_graphical = true;

    m_rgb.resize( 3 );
    m_pin.resize( 2 );

    m_clockPin = new IoPin( 180, QPoint(-4-8,0), id+"-InPin", 0, this, input );
    m_pin[0] = m_clockPin;

    m_output = new IoPin( 0, QPoint(4+8,0), id+"-OutPin", 1, this, output );
    m_pin[1] = m_output;

    setClockInv( false );                       //Don't Invert Clock pin

    m_cols = 1;
    setRows( 1 );

    Simulator::self()->addToUpdateList( this );
}
WS2812::~WS2812()
{
    Simulator::self()->remFromUpdateList( this );
}

QList<propGroup_t> WS2812::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Rows", tr("Rows"),""} );
    mainGroup.propList.append( {"Cols", tr("Columns"),""} );
    return {mainGroup};
}

void WS2812::updateStep()
{
    update();
}

void WS2812::initialize()
{
    for( int i=0; i<m_leds; i++ ) m_led[i] = QColor( 0, 0, 0 );
    m_lastTime = 0;
    m_data = 0;
    m_word = 0;
    m_byte = 0;
    m_bit  = 7;
}
void WS2812::voltChanged()
{
    int clkState = eClockedDevice::getClockState(); // Get Clk to don't miss any clock changes

    uint64_t CircTime = Simulator::self()->circTime();
    uint64_t time = CircTime - m_lastTime;
    m_lastTime = CircTime;

    if( clkState == Clock_Rising )               // Input Rising edge
    {
        if( time > 50*1e6 )                      // Time > 50uS -> Reset
        {
            m_data = 0;
            m_word = 0;
            m_byte = 0;
            m_bit  = 7;
        }
        else if(( time > 400*1e3 )&&( time < 951*1e3 )) // Valid L State Time
        {
            if( m_word >= m_leds )
            {
                m_output->setOutState( true );
            }
            else if( m_newWord )
            {
                if( m_lastHstate  )
                {
                    if( time < 751*1e3 )  saveBit( 1 ); // Valid bit = 1
                }
                else if( time > 649 *1e3) saveBit( 0 ); // Valid bit = 0
            }
        }
        m_newWord = true;
    }
    else if( clkState == Clock_Falling )               // Input Falling edge
    {
        if( m_word >= m_leds )
        {
            m_output->setOutState( false );
        }
        else if(( time > 199*1e3 )&&( time < 851*1e3 )) // Valid H State Time
        {
            if( time > 500*1e3 ) m_lastHstate = true;
            else                 m_lastHstate = false;

            if(( m_byte == 2 )&&( m_bit == 0 )) // Low time for last bit can be anything?
            {                                   // so save it here and skip saving
                saveBit( m_lastHstate );        // at next Rising Edge Using m_newWord
                m_newWord = false;
            }
        }
    }
}

void WS2812::saveBit( bool bit )
{
    if( bit ) m_data |=   (1 << m_bit);
    else      m_data &=  ~(1 << m_bit);

    if( --m_bit < 0 )
    {
        m_rgb[ m_byte ] = m_data;

        m_data = 0;
        m_bit  = 7;

        if( ++m_byte > 2 )
        {
            m_led[ m_word ] = QColor( m_rgb[1], m_rgb[0], m_rgb[2] );

            m_byte = 0;

            m_word++;
        }
    }
}

int WS2812::rows()
{
    return m_rows;
}

void WS2812::setRows( int rows )
{
    if( rows == m_rows ) return;
    if( rows < 1 ) rows = 1;
    m_rows = rows;
    updateLeds();
}

int WS2812::cols()
{
    return m_cols;
}

void WS2812::setCols( int cols )
{
    if( cols == m_cols ) return;
    if( cols < 1 ) cols = 1;
    m_cols = cols;
    updateLeds();
}

void WS2812::updateLeds()
{
    m_leds = m_rows*m_cols;
    m_led.resize( m_leds );
    m_area = QRect( -6, -6, m_cols*12, m_rows*12 );
    m_pin[1]->setPos( 12*m_cols, 12*m_rows-12);
    Circuit::self()->update();
}

void WS2812::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( QColor(Qt::black) );

    p->drawRect( m_area );

    if( !m_hidden )
    {
        QPointF points[3] = {
        QPointF( -8,-2 ),
        QPointF( -8+3,0 ),
        QPointF( -8, 2 )     };
        p->drawPolygon(points, 3);
    }

    for( int row=0; row<m_rows; row++ )
    {
        for( int col=0; col<m_cols; col++ )
        {
            p->setBrush( m_led[row*m_cols+col] );
            p->drawEllipse( -6+col*12, -6+row*12, 12, 12 );
        }
    }
}

#include "moc_ws2812.cpp"

