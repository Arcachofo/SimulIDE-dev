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
    m_numCons = 0;

    initialize();

    Simulator::self()->addToEnodeList( this );
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

void eNode::stampCurrent( ePin* epin, double data )
{
    if( m_nodeList.value( epin ) == m_nodeNum  )
        return; // Be sure msg doesn't come from this node

    m_currList[epin] = data;
    m_currChanged = true;

    if( !m_changed ){
        m_changed = true;
        Simulator::self()->addToChangedNodes( this );
}   }

void eNode::stampAdmitance( ePin* epin, double data )
{
    if( m_nodeList.value( epin ) == m_nodeNum  )
        return; // Be sure msg doesn't come from this node

    m_admitList[epin] = data;
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
        m_admit.clear();
        m_totalAdmit = 0;

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
        if( !m_single || m_switched ) stampAdmit();
        m_admitChanged = false;
    }
    if( m_currChanged )
    {
        m_totalCurr  = 0;
        for( double current : m_currList ) m_totalCurr += current;

        if( !m_single || m_switched ) stampCurr();
        m_currChanged  = false;
    }
    if( m_single ) solveSingle();
}

void eNode::stampAdmit()
{
    //int nonCero = 0;
    QHashIterator<int, double> ai(m_admit); // iterate admitance hash: eNode-Admit
    while( ai.hasNext() )
    {
        ai.next();
        int enode = ai.key();
        double admit = ai.value();
        if( enode>0 ) CircMatrix::self()->stampMatrix( m_nodeNum, enode, -admit );

        if( m_switched )                       // Find Open/Close events
        {
            //if( admit > 0 ) nonCero++;
            double admitP = m_admitPrev[enode];

            if(( admit != admitP )             // Open/Close event found
              &&((admit==0)||(admitP==0))) CircMatrix::self()->setCircChanged();
    }   }
    if( m_switched )
    {
        m_admitPrev = m_admit;
        //if( nonCero < 2 ) m_totalAdmit += 1e-12; //pnpBias example error
    }
    CircMatrix::self()->stampMatrix( m_nodeNum, m_nodeNum, m_totalAdmit );
}

void eNode::stampCurr()
{
    CircMatrix::self()->stampCoef( m_nodeNum, m_totalCurr );
}

void eNode::solveSingle()
{
    double volt = 0;

    if( m_totalAdmit > 0 ) volt = m_totalCurr/m_totalAdmit;
    setVolt( volt );
}

void eNode::addConnection( ePin* epin, int enodeComp ) // Add node at other side of pin
{
    m_nodeList[epin] = enodeComp;
}

QList<int> eNode::getConnections()
{
    QList<int> cons;
    for( int nodeNum : m_nodeList )
    { if( m_admit.value( nodeNum ) > 0 ) cons.append( nodeNum ); }
    return cons;
}

void  eNode::setVolt( double v )
{
    if( m_volt != v ) //( fabs(m_volt-v) > 1e-9 ) //
    {
        m_voltChanged = true; // Used for wire animation
        m_volt = v;

        for( eElement* el : m_changedFast )
        {
            if( el->added ) continue;
            Simulator::self()->addToChangedFast( el );
            el->added = true;
        }
        for( eElement* el : m_nonLinear )
        {
            Simulator::self()->addToNoLinList( el );
}   }   }

void eNode::addEpin( ePin* epin )
{ if( !m_ePinList.contains(epin)) m_ePinList.append(epin); }

void eNode::remEpin( ePin* epin )
{
    if( m_ePinList.contains(epin) ) m_ePinList.removeOne( epin );
    if( m_nodeList.contains(epin) ) m_nodeList.remove( epin );
    //if( m_ePinList.isEmpty() ) // If No epins then remove this enode
    //    Simulator::self()->remFromEnodeList( this, true );
}

void eNode::clear()
{
    for( ePin* epin : m_ePinList )
    {
        epin->setEnode( NULL );
        epin->setEnodeComp( NULL );
    }
}

void eNode::voltChangedCallback( eElement* el )
{ if( !m_changedFast.contains(el) ) m_changedFast.append(el); }

void eNode::remFromChangedCallback( eElement* el )
{  m_changedFast.removeOne(el); }

void eNode::addToNoLinList( eElement* el )
{ if( !m_nonLinear.contains(el) ) m_nonLinear.append(el); }

void eNode::remFromNoLinList( eElement* el )
{ m_nonLinear.removeOne(el); }
