/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "e-pin.h"
#include "e-node.h"
#include "circuit.h"


ePin::ePin( QString id, int index )
{
    m_id    = id;
    m_index = index;
    m_state = 0;
    m_enode    = 0l;
    m_enodeCon = 0l;
    m_connected = false;
    m_inverted  = false;
}
ePin::~ePin()
{
    if( m_enode ) m_enode->remEpin( this );
}

void ePin::reset()
{
    setEnode( 0l );
}

eNode* ePin::getEnode()
{
    return m_enode; 
}

void ePin::setEnode( eNode* enode )
{
    if( enode == m_enode ) return;

    if( m_enode ) m_enode->remEpin( this );
    if( enode )   enode->addEpin( this );

    m_enode = enode;
    m_connected = (enode!=0l);
}

void ePin::setEnodeComp( eNode* enode )
{
    m_enodeCon = enode;
    int enodeConNum = 0;
    if( enode ) enodeConNum = enode->getNodeNumber();
    if( m_connected )
        m_enode->pinChanged( this, enodeConNum );
}

void ePin::stampCurrent( double data )
{
    if( m_connected ) m_enode->stampCurrent( this, data );
}

void ePin::stampAdmitance( double data )
{
    if( m_connected )
    {
        if( !m_enodeCon )
            data = 1e-12;
        m_enode->stampAdmitance( this, data );
    }
}

double ePin::getVolt()
{
    if( m_connected )return m_enode->getVolt();
    if( m_enodeCon ) return m_enodeCon->getVolt();
    return 0;
}

void ePin::setConnected( bool connected )  { m_connected = connected; }

bool ePin::isConnected() { return m_connected; }

eNode* ePin::getEnodeComp(){ return m_enodeCon; }

bool ePin::inverted() { return m_inverted; }

void ePin::setInverted( bool inverted ){ m_inverted = inverted; }

QString ePin::getId() { return m_id; }

void ePin::setId( QString id )
{
    Circuit::self()->updatePin( this, id );
    m_id = id;
}


