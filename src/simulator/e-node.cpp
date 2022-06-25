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

#include "e-node.h"
#include "e-pin.h"
#include "e-element.h"
#include "circmatrix.h"
#include "simulator.h"

eNode::eNode( QString id )
{
    m_id = id;
    m_nodeNum = 0;

    initialize();

    if( !id.isEmpty() ) Simulator::self()->addToEnodeList( this );
}
eNode::~eNode(){}

void eNode::initialize()
{
    m_switched     = false;
    m_single       = false;
    m_changed      = false;
    m_currChanged  = false;
    m_admitChanged = false;
    nextCH = NULL;

    m_changedFast.clear();
    m_nonLinear.clear();
    m_admitList.clear();
    m_currList.clear();
    m_nodeList.clear();

    m_volt = 0;
}

void eNode::stampCurrent( ePin* epin, double current )
{
    if( m_nodeList.value( epin ) == m_nodeNum  )
        return; // Be sure msg doesn't come from this node

    m_currList[epin] = current;
    m_currChanged = true;

    if( !m_changed ){
        m_changed = true;
        Simulator::self()->addToChangedNodes( this );
}   }

void eNode::stampAdmitance( ePin* epin, double admit )
{
    if( m_nodeList.value( epin ) == m_nodeNum  )
        return; // Be sure msg doesn't come from this node

    m_admitList[epin] = admit;
    if( admit == 0 ) m_switched = true;
    m_admitChanged = true;

    if( !m_changed ){
        m_changed = true;
        Simulator::self()->addToChangedNodes( this );
}   }

void eNode::stampMatrix()
{
    if( m_nodeNum == 0 ) return;
    m_changed = false;

    if( m_admitChanged )
    {
        m_totalAdmit = 0;
        if( m_switched ) m_totalAdmit += 1e-12; // Weak connection to ground

        if( m_single ){
            for( double adm : m_admitList ) m_totalAdmit += adm;
        }
        else{
            m_admit.clear();
            QHashIterator<ePin*, double> i(m_admitList); // ePin-Admit
            while( i.hasNext() )
            {
                i.next();
                double adm = i.value();
                ePin* epin = i.key();
                int enode = m_nodeList.value( epin );

                m_admit[enode] += adm;
                m_totalAdmit   += adm;
            }
            QHashIterator<int, double> ai(m_admit); // eNode-Admit
            while( ai.hasNext() )
            {
                ai.next();
                int    enode = ai.key();
                double admit = ai.value();
                if( enode>0 ) CircMatrix::self()->stampMatrix( m_nodeNum, enode, -admit );
            }
            CircMatrix::self()->stampMatrix( m_nodeNum, m_nodeNum, m_totalAdmit );
        }
        m_admitChanged = false;
    }
    if( m_currChanged )
    {
        m_totalCurr  = 0;
        for( double current : m_currList ) m_totalCurr += current;

        if( !m_single ) CircMatrix::self()->stampCoef( m_nodeNum, m_totalCurr );
        m_currChanged  = false;
    }
    if( m_single ) solveSingle();
}

void eNode::solveSingle()
{
    double volt = 0;
    if( m_totalAdmit > 0 ) volt = m_totalCurr/m_totalAdmit;
    setVolt( volt );
}

void  eNode::setVolt( double v )
{
    if( m_volt != v )
    {
        m_voltChanged = true; // Used for wire animation
        m_volt = v;

        for( eElement* el : m_changedFast )
        {
            if( el->added ) continue;
            Simulator::self()->addToChangedList( el );
            el->added = true;
        }
        for( eElement* el : m_nonLinear )
        {
            if( el->added ) continue;
            Simulator::self()->addToNoLinList( el );
            el->added = true;
        }
}   }

void eNode::addEpin( ePin* epin )
{ if( !m_ePinList.contains(epin)) m_ePinList.append(epin); }

void eNode::remEpin( ePin* epin )
{
    if( m_ePinList.contains(epin) ) m_ePinList.removeOne( epin );
    if( m_nodeList.contains(epin) ) m_nodeList.remove( epin );
    if( m_admitList.contains(epin) ) m_admitList.remove( epin );
}

void eNode::clear()
{
    for( ePin* epin : m_ePinList )
    {
        epin->setEnode( NULL );
        epin->setEnodeComp( NULL );
    }
}

void eNode::addConnection( ePin* epin, int enodeComp ) // Add node at other side of pin
{ m_nodeList[epin] = enodeComp; }

QList<int> eNode::getConnections()
{ return m_nodeList.values(); }

void eNode::voltChangedCallback( eElement* el )
{ if( !m_changedFast.contains(el) ) m_changedFast.append(el); }

void eNode::remFromChangedCallback( eElement* el )
{  m_changedFast.removeOne(el); }

void eNode::addToNoLinList( eElement* el )
{ if( !m_nonLinear.contains(el) ) m_nonLinear.append(el); }

void eNode::remFromNoLinList( eElement* el )
{ m_nonLinear.removeOne(el); }
