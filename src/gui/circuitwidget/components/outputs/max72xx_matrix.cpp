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

#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "max72xx_matrix.h"

static const char* Max72xx_matrix_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","NumDisplays")
};

Component* Max72xx_matrix::construct( QObject* parent, QString type, QString id )
{
    return new Max72xx_matrix( parent, type, id );
}

LibraryItem* Max72xx_matrix::libraryItem()
{
    return new LibraryItem(
            tr( "Max72xx_matrix" ),
            tr( "Displays" ),
            "ledmatrix.png",
            "Max72xx_matrix",
            Max72xx_matrix::construct);
}

Max72xx_matrix::Max72xx_matrix( QObject* parent, QString type, QString id )
         : Component( parent, type, id )
         , eLogicDevice( (id+"-eElement") )
{
    Q_UNUSED( Max72xx_matrix_properties );

    m_graphical = true;

    m_numDisplays = 4;

    m_area = QRectF(-36, -44, 4+64*m_numDisplays+4, 88 );

    m_pinCS = new Pin( 270, QPoint(-12, 52), id+"PinCS", 0, this );
    m_pinDin = new Pin( 270, QPoint(-20, 52), id+"PinDin", 0, this );
    m_pinSck = new Pin( 270, QPoint(-28, 52), id+"PinSck", 0, this );

    m_pinCS->setLabelText(  " CS" );
    m_pinDin->setLabelText( " DIN" );
    m_pinSck->setLabelText( " SCK" );

    m_pin.resize( 3 );
    m_pin[0] = m_pinCS;
    m_pin[1] = m_pinDin;
    m_pin[2] = m_pinSck;

    eLogicDevice::createInput( m_pinCS );
    eLogicDevice::createInput( m_pinDin );
    eLogicDevice::createClockPin( m_pinSck );

    Simulator::self()->addToUpdateList( this );

    setLabelPos(-32, -58, 0);
    setShowId( true );

    initialize();
}

Max72xx_matrix::~Max72xx_matrix()
{
}

void Max72xx_matrix::stamp()
{
    eNode* enode = m_pinSck->getEnode();    // Register for Sck changes callback
    if( enode ) enode->voltChangedCallback( this );

    enode = m_pinCS->getEnode();            // Register for CS changes callback
    if( enode ) enode->voltChangedCallback( this );
}

void Max72xx_matrix::initialize()
{
    for( int i = 0; i < 11; i++)
    {
        for( int j = 0; j < 8; j++)
            m_ram[i][j] = 0;
        m_intensity[i] = 0;
    }
    m_decodemode = 0;
    m_scanlimit = 0;
    m_shutdown = true;
    m_test = false;

    m_rxReg = 0;
    m_inBit = 0;
    m_inDisplay = 0;

    updateStep();
}

void Max72xx_matrix::voltChanged()
{
    if( m_pinCS->getVolt()>1.6 )            // CS high: not selected
    {
        m_rxReg = 0;
        m_inBit  = 0;
        m_inDisplay = 0;
        return;
    }
    if( eLogicDevice::getClockState() != Clock_Rising ) return;

    m_rxReg &= ~1;

    if( m_pinDin->getVolt()>1.6 ) m_rxReg |= 1;

    if( m_inBit == 15 )
    {
        proccessCommand();
        m_inBit = 0;
        m_inDisplay++;
    }
    else
    {
        m_rxReg <<= 1;
        m_inBit++;
    }
}

void Max72xx_matrix::updateStep()
{
    update();
}

void Max72xx_matrix::remove()
{
    Simulator::self()->remFromUpdateList( this );

    Component::remove();
}

void Max72xx_matrix::proccessCommand()
{
    if ( m_inDisplay >= 11 ) return;

    int addr = (m_rxReg>>8) & 0x0F;

    switch( addr )
    {
        case 0:  // No-Op
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:  // Digits 0 to 7
            m_ram[m_inDisplay][addr-1] = m_rxReg & 0xFF;
            break;
        case 9:  // Decode mode (only no-decode 0 mode supported)
            m_decodemode = m_rxReg & 0xFF;
            break;
        case 10: // Intensity (0-15)
            m_intensity[m_inDisplay] = m_rxReg & 0x0F;
            break;
        case 11: // Scan limit (0-7)
            m_scanlimit = m_rxReg & 0x07;
            break;
        case 12: // Shutdown
            m_shutdown = !(m_rxReg & 0x01);
            break;
        case 15: // Display test
            m_test = m_rxReg & 0x01;
            break;
    }
}

int Max72xx_matrix::numDisplays()
{
    return m_numDisplays;
}

void Max72xx_matrix::setNumDisplays( int displays )
{
    if( displays == m_numDisplays ) return;
    if( displays < 1 ) displays = 1;
    if( displays > 11 ) displays = 11;
    m_numDisplays = displays;

    hide();
    m_area = QRectF(-36, -44, 4+64*m_numDisplays+4, 88 );
    show();
}

void Max72xx_matrix::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option); Q_UNUSED(widget);

    Component::paint( p, option, widget );

    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );

    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area, 2, 2 );

    QBrush brush_off( Qt::black );
    for( int row = 0; row < 8; row++ )
    {
        int x = -32;
        int y = -40+row*8;
        for( int display = 0; display < m_numDisplays; display++)
        {
            QColor color = Qt::yellow;
            int factor = 100 + (15-m_intensity[display])*3;
            QBrush brush_on( color.darker(factor) );
            for( int col = 0; col < 8; col++)
            {
                int bit = m_ram[display][row] & (0x80>>col);
                if( !m_shutdown && (m_test || ((m_decodemode == 0) && (row <= m_scanlimit) && bit)) )
                    p->setBrush(brush_on);
                else
                    p->setBrush(brush_off);
                p->drawEllipse(x+1, y+1, 6, 6);
                x += 8;
            }
        }
    }
}

#include "moc_max72xx_matrix.cpp"

