/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "mux_analog.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "e-resistor.h"
#include "e-node.h"
#include "iopin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* MuxAnalog::construct( QObject* parent, QString type, QString id )
{ return new MuxAnalog( parent, type, id ); }

LibraryItem* MuxAnalog::libraryItem()
{
    return new LibraryItem(
        tr( "Analog Mux" ),
        tr( "Active" ),
        "1to3-c.png",
        "MuxAnalog",
        MuxAnalog::construct );
}

MuxAnalog::MuxAnalog( QObject* parent, QString type, QString id )
         : LogicComponent( parent, type, id )
{
    setLabelPos(-16,-16, 0 );
    setValLabelPos(-16,-16-10, 0 );
    
    m_inputPin = new Pin( 180, QPoint( -24, 8 ), m_id+"-PinInput", 0, this );
    m_inputPin->setLabelText( " Z" );
    m_inputPin->setLabelColor( QColor( 0, 0, 0 ) );
    
    m_enablePin = new Pin( 180, QPoint( -24, 16 ), m_id+"-PinEnable", 0, this );
    m_enablePin->setLabelText( " En" );
    m_enablePin->setLabelColor( QColor( 0, 0, 0 ) );
    m_enablePin->setInverted( true );
    
    m_admit = 0.01;
    m_addrBits = 0;
    m_channels = 0;
    setAddrBits( 3 );

    addPropGroup( { tr("Main"), {
new IntProp<MuxAnalog>( "Address_Bits", tr("Address Size"),"_Bits", this, &MuxAnalog::addrBits,  &MuxAnalog::setAddrBits, "uint" ),
new DoubProp<MuxAnalog>( "Impedance"  , tr("Impedance")   ,"Ω"    , this, &MuxAnalog::impedance, &MuxAnalog::setImpedance ),
    }} );
}
MuxAnalog::~MuxAnalog(){}

void MuxAnalog::stamp()
{
    eNode* enode = m_inputPin->getEnode();
    if( enode ) enode->setSwitched( true );

    for( int i=0; i<m_channels; ++i )
    {
        m_ePin[i]->setEnode( enode );

       eNode* node = m_chanPin[i]->getEnode();
       if( node ) node->setSwitched( true );
    }
    m_enablePin->changeCallBack( this );
    for( Pin* pin : m_addrPin ) pin->changeCallBack( this );

    m_enabled = false;
}

void MuxAnalog::voltChanged()
{
    m_enabled = m_enablePin->getVolt() < 2.5;

    int address = 0;
    for( int i=0; i<m_addrBits; ++i )
    {
        bool state = (m_addrPin[i]->getVolt()>2.5);
        if( state ) address += pow( 2, i );
    }
    m_address = address;

    Simulator::self()->addEvent( m_propDelay, this );
}

void MuxAnalog::runEvent()
{
    for( int i=0; i<m_channels; ++i )
    {
        if( m_enabled && (i == m_address) )
        {    if( m_resistor[i]->admit() == 0 ) m_resistor[i]->setAdmit( m_admit ); }
        else if( m_resistor[i]->admit() != 0 ) m_resistor[i]->setAdmit( 0 );
}   }

void MuxAnalog::setAddrBits( int bits )
{
    if( bits == m_addrBits ) return;
    if( bits < 1 ) bits = 1;
    
    int channels = pow( 2, bits );
    
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    
    if( bits < m_addrBits ) deleteAddrBits( m_addrBits-bits );
    else                    createAddrBits( bits-m_addrBits );

    if  ( channels < m_channels ) deleteResistors( m_channels-channels );
    else                          createResistors( channels-m_channels );
    
    m_channels = channels;
    
    int rside = m_channels*8+8;
    int size = 5*8 + bits*8;
    if( rside > size ) size = rside;

    m_area = QRect( -2*8, 0, 4*8, size );

    m_enablePin->setPos( QPoint(-3*8,4*8+bits*8 ) );
    m_enablePin->isMoved();
    m_enablePin->setLabelPos();

    Circuit::self()->update();
}

void MuxAnalog::createAddrBits( int c )
{
    int start = m_addrBits;
    m_addrBits = m_addrBits+c;
    m_addrPin.resize( m_addrBits );

    for( int i=start; i<m_addrBits; i++ )
    {
        m_addrPin[i] = new Pin( 180, QPoint(-3*8,3*8+i*8 ), m_id+"-pinAddr"+QString::number(i), 0, this);
        m_addrPin[i]->setLabelText( " A"+QString::number(i) );
        m_addrPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
}   }

void MuxAnalog::deleteAddrBits( int d )
{
    int start = m_addrBits-d;

    for( int i=start; i<m_addrBits; i++ )
    {
        m_addrPin[i]->removeConnector();
        delete m_addrPin[i];
    }
    m_addrBits = m_addrBits-d;
    m_addrPin.resize( m_addrBits );
}

void MuxAnalog::createResistors( int c )
{
    int start = m_channels;
    m_channels = m_channels+c;
    m_resistor.resize( m_channels );
    m_chanPin.resize( m_channels );
    m_ePin.resize( m_channels );

    for( int i=start; i<m_channels; i++ )
    {
        QString reid = m_id+"-resistor"+QString::number(i);
        m_resistor[i] = new eResistor( reid );
        m_ePin[i]     = new ePin( reid+"-pinL", 0 );
        m_resistor[i]->setEpin( 0, m_ePin[i] );
        
        m_chanPin[i] = new Pin( 0, QPoint( 3*8, 8+i*8 ), m_id+"-pinY"+QString::number(i), 0, this);
        m_chanPin[i]->setLabelText( "Y"+QString::number(i)+" " );
        m_chanPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        m_resistor[i]->setEpin( 1, m_chanPin[i] );

        m_resistor[i]->setAdmit( 0 );
}   }

void MuxAnalog::deleteResistors( int d )
{
    int start = m_channels-d;

    for( int i=start; i<m_channels; ++i )
    {
        m_chanPin[i]->removeConnector();
        delete m_chanPin[i];
        delete m_ePin[i];
        delete m_resistor[i];
    }
    m_resistor.resize( start );
    m_chanPin.resize( start );
    m_ePin.resize( start );
}

void MuxAnalog::remove()
{
    m_inputPin->removeConnector();
    m_enablePin->removeConnector();
    for( Pin* pin :m_addrPin  ) pin->removeConnector();

    deleteResistors( m_channels );
    deleteAddrBits( m_addrBits );
    
    Component::remove();
}

void MuxAnalog::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawRoundRect( m_area, 4, 4 );
}
