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

#include "flipflopbase.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

#include "boolprop.h"
#include "stringprop.h"

FlipFlopBase::FlipFlopBase( QObject* parent, QString type, QString id )
            : LogicComponent( parent, type, id )
{
    m_dataPins = 0;

    addPropGroup( { tr("Main"), {
new BoolProp<FlipFlopBase>( "Clock_Inverted", tr("Clock Inverted")      ,"", this, &FlipFlopBase::clockInv, &FlipFlopBase::setClockInv ),
new BoolProp<FlipFlopBase>( "Reset_Inverted", tr("Set / Reset Inverted"),"", this, &FlipFlopBase::srInv,    &FlipFlopBase::setSrInv ),
new StringProp<FlipFlopBase>( "Trigger"     , tr("Trigger Type")        ,"", this, &FlipFlopBase::triggerStr, &FlipFlopBase::setTriggerStr, "enum" ),
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
FlipFlopBase::~FlipFlopBase(){}

void FlipFlopBase::stamp()
{
    m_Q0 = 0;
    m_setPin->changeCallBack( this );
    m_resetPin->changeCallBack( this );

    if( m_trigger != Clock ) // J K or D
    { for( int i=0; i<m_dataPins; i++ ) m_inPin[i]->changeCallBack( this ); }

    LogicComponent::stamp();
    m_outPin[1]->setOutState( true );
}

void FlipFlopBase::setSrInv( bool inv )
{
    m_srInv = inv;
    m_setPin->setInverted( inv );   // Set
    m_resetPin->setInverted( inv ); // Reset

    Circuit::self()->update();
}
