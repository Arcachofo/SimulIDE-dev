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

#include "flipflopjk.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"


Component* FlipFlopJK::construct( QObject* parent, QString type, QString id )
{
        return new FlipFlopJK( parent, type, id );
}

LibraryItem* FlipFlopJK::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop JK" ),
        tr( "Logic/Memory" ),
        "3to2.png",
        "FlipFlopJK",
        FlipFlopJK::construct );
}

FlipFlopJK::FlipFlopJK( QObject* parent, QString type, QString id )
          : LogicComponent( parent, type, id )
          , eFlipFlopJK( id )
{
    m_width  = 3;
    m_height = 4;
    
    QStringList pinList;

    pinList // Inputs:
            << "IL01 J"
            << "IL03 K"
            << "IU01S"
            << "ID02R"
            << "IL02>"
            
            // Outputs:
            << "OR01Q"
            << "OR03!Q"
            ;
    init( pinList );
    
    eLogicDevice::createInput( m_inPin[0] );                  // Input J
    eLogicDevice::createInput( m_inPin[1] );                  // Input K
    eLogicDevice::createInput( m_inPin[2] );                  // Input S
    eLogicDevice::createInput( m_inPin[3] );                  // Input R
    
    m_trigPin = m_inPin[4];
    eLogicDevice::createClockPin( m_trigPin );            // Input Clock
    
    eLogicDevice::createOutput( m_outPin[0] );               // Output Q
    eLogicDevice::createOutput( m_outPin[1] );               // Output Q'

    setSrInv( true );                         // Invert Set & Reset pins
    setClockInv( false );                       //Don't Invert Clock pin
    setTrigger( Clock );
}
FlipFlopJK::~FlipFlopJK(){}

QList<propGroup_t> FlipFlopJK::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Clock_Inverted", tr("Clock Inverted"),""} );
    mainGroup.propList.append( {"S_R_Inverted", tr("Set / Reset Inverted"),""} );
    mainGroup.propList.append( {"Trigger", tr("Trigger Type"),"enum"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void FlipFlopJK::setTrigger( Trigger trigger )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    int trig = static_cast<int>( trigger );
    eLogicDevice::seteTrigger( trig );
    LogicComponent::setTrigger( trigger );

    Circuit::self()->update();
}

#include "moc_flipflopjk.cpp"
