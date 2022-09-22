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

#include "adc.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* ADC::construct( QObject* parent, QString type, QString id )
{ return new ADC( parent, type, id ); }

LibraryItem* ADC::libraryItem()
{
    return new LibraryItem(
        tr( "ADC" ),
        tr( "Logic/Other Logic" ),
        "1to3.png",
        "ADC",
        ADC::construct );
}

ADC::ADC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 9;

    setLabelPos(-16,-80, 0);
    setNumOuts( 8 );    // Create Output Pins
    setNumInps( 1, "In" );
    m_maxVolt = 5;

    addPropGroup( { tr("Main"), {
new IntProp<ADC>(  "Num_Bits", tr("Size")             ,"_Bits", this, &ADC::numOuts, &ADC::setNumOuts, "uint" ),
new DoubProp<ADC>( "Vref"    , tr("Reference Voltage"),"V"    , this, &ADC::maxVolt, &ADC::setMaxVolt ),
    }} );
    addPropGroup( { tr("Electric"), IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
ADC::~ADC(){}

void ADC::stamp()
{
    m_inPin[0]->changeCallBack( this );
    LogicComponent::stamp();
}

void ADC::voltChanged()
{
    double volt = m_inPin[0]->getVoltage();
    m_nextOutVal = volt*m_maxValue/m_maxVolt+0.1;
    LogicComponent::sheduleOutPuts( this );
}

void ADC::setNumOuts( int outs )
{
    if( outs < 1 ) return;
    m_maxValue = pow( 2, outs )-1;
    IoComponent::setNumOuts( outs, "D" );
    IoComponent::setNumInps( 1, "In" );
}
