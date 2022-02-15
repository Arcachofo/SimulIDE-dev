/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include <math.h>

#include "dac.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* DAC::construct( QObject* parent, QString type, QString id )
{ return new DAC( parent, type, id ); }

LibraryItem* DAC::libraryItem()
{
    return new LibraryItem(
        tr( "DAC" ),
        tr( "Logic/Other Logic" ),
        "3to1.png",
        "DAC",
        DAC::construct );
}

DAC::DAC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
{    
    m_width  = 4;
    m_height = 9;

    setLabelPos(-16,-80, 0);
    setNumInps( 8 );       // Create Input Pins
    setNumOuts( 1, "Out" );
    m_maxVolt = 5;

    addPropGroup( { tr("Main"), {
new IntProp<DAC>(  "Num_Bits", tr("Size")             ,"_Bits", this, &DAC::numInps, &DAC::setNumInps, "uint" ),
new DoubProp<DAC>( "Vref"    , tr("Reference Voltage"),"V"    , this, &DAC::maxVolt, &DAC::setMaxVolt )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
DAC::~DAC(){}

void DAC::stamp()
{
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this );

    m_outPin[0]->setOutState( true );
    m_val = -1;
}

void DAC::voltChanged()
{
    m_val = 0;

    for( uint i=0; i<m_inPin.size(); ++i )
        if( m_inPin[i]->getInpState() ) m_val += pow( 2, i );

    Simulator::self()->addEvent( m_propDelay, this );
}

void DAC::runEvent()
{
    double v = m_maxVolt*m_val/m_maxValue;

    m_outPin[0]->setOutHighV( v );
    m_outPin[0]->setOutState( true );
}

void DAC::setNumInps( int inputs )
{
    if( inputs < 1 ) return;
    m_maxValue = pow( 2, inputs )-1;
    IoComponent::setNumInps( inputs, "D" );
    IoComponent::setNumOuts( 1, "Out" );
}
