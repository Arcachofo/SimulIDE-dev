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

FlipFlopBase::FlipFlopBase( QObject* parent, QString type, QString id )
         : LogicComponent( parent, type, id )
         , eElement( id )
{
    m_dataPins = 0;
}
FlipFlopBase::~FlipFlopBase(){}

QList<propGroup_t> FlipFlopBase::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Clock_Inverted", tr("Clock Inverted"),""} );
    mainGroup.propList.append( {"S_R_Inverted", tr("Set / Reset Inverted"),""} );
    mainGroup.propList.append( {"Trigger", tr("Trigger Type"),"enum"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void FlipFlopBase::stamp()
{
    m_Q0 = 0;
    eNode* enode = m_setPin->getEnode();         // Set pin
    if( enode ) enode->voltChangedCallback( this );

    enode = m_resetPin->getEnode();              // Reset pin
    if( enode ) enode->voltChangedCallback( this );

    if( m_etrigger != Trig_Clk )
    {
        for( int i=0; i<m_dataPins; i++ ) // J K or D
        {
            eNode* enode = m_inPin[i]->getEnode();
            if( enode ) enode->voltChangedCallback( this );
        }
    }
    LogicComponent::stamp( this );
    m_outPin[0]->setOutState( true );
}

void FlipFlopBase::setSrInv( bool inv )
{
    m_srInv = inv;
    m_setPin->setInverted( inv ); // Set
    m_resetPin->setInverted( inv ); // Reset

    Circuit::self()->update();
}

#include "moc_flipflopbase.cpp"
