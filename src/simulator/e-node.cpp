/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "e-node.h"
#include "pin.h"
#include "e-pin.h"
#include "connector.h"
#include "e-element.h"
#include "circmatrix.h"
#include "simulator.h"

eNode::eNode( QString id )
{
    m_id = id;
    m_nodeNum = 0;

    m_voltChEl     = nullptr;
    m_nonLinEl     = nullptr;
    m_firstAdmit   = nullptr;
    m_firstSingAdm = nullptr;
    m_firstCurrent = nullptr;
    m_nodeAdmit    = nullptr;

    if( !id.isEmpty() ) Simulator::self()->addToEnodeList( this );
}
eNode::~eNode()
{
    clearElmList( m_voltChEl );
    clearElmList( m_nonLinEl );
    clearConnList( m_firstAdmit );
    clearConnList( m_firstSingAdm );
    clearConnList( m_firstCurrent );
    clearConnList( m_nodeAdmit );
}

void eNode::initialize()
{
    m_voltChanged  = true; // Used for wire animation
    //m_switched     = false;
    m_single       = false;
    m_changed      = false;
    m_currChanged  = false;
    m_admitChanged = false;
    m_nodeGroup = -1;
    nextCH = nullptr;
    m_volt = 0;

    clearElmList( m_voltChEl );
    m_voltChEl = nullptr;

    clearElmList( m_nonLinEl );
    m_nonLinEl = nullptr;

    clearConnList( m_firstAdmit );
    m_firstAdmit = nullptr;

    clearConnList( m_firstSingAdm );
    m_firstSingAdm = nullptr;

    clearConnList( m_firstCurrent );
    m_firstCurrent = nullptr;

    clearConnList( m_nodeAdmit );
    m_nodeAdmit = nullptr;

    m_nodeList.clear();
}

void eNode::addConnection( ePin* epin, int node )
{
    if( node == m_nodeNum ) return;// Be sure msg doesn't come from this node

    Connection* first = m_firstAdmit; // Create list of connections
    while( first ){
        if( first->epin == epin ) return; // Connection already in the list
        first = first->next;
    }
    Connection* conn = new Connection( epin, node );
    conn->next = m_firstAdmit;  // Prepend
    m_firstAdmit = conn;

    first = m_nodeAdmit;            // Create list of admitances to nodes (reusing Connection class)
    while( first ){
        if( first->node == node ) return; // Node already in the list
        first = first->next;
    }
    conn = new Connection( epin, node );
    conn->next = m_nodeAdmit;  // Prepend
    m_nodeAdmit = conn;

    if( !m_nodeList.contains( node ) ) m_nodeList.append( node ); // Used by CircMatrix
}

void eNode::stampAdmitance( ePin* epin, double admit ) // Be sure msg doesn't come from this node
{
    Connection* conn = m_firstAdmit;
    while( conn ){
        if( conn->epin == epin ) { conn->value = admit; break; } // Connection found
        conn = conn->next;
    }

    //if( admit == 0 ) m_switched = true;
    m_admitChanged = true;
    changed();
}

void eNode::addSingAdm( ePin* epin, int node, double admit )
{
    Connection* conn = new Connection( epin, node );
    conn->next = m_firstSingAdm;  // Prepend
    m_firstSingAdm = conn;
    conn->value = admit;

    conn = nullptr;
    Connection* first = m_nodeAdmit;   // Create list of admitances to nodes (reusing Connection class)
    while( first ){
        if( first->node == node )  { conn = first; break; }// Node already in the list
        first = first->next;
    }
    if( !conn ){
        conn = new Connection( epin, node );
        conn->next = m_nodeAdmit;  // Prepend
        m_nodeAdmit = conn;
    }
    if( !m_nodeList.contains( node ) ) m_nodeList.append( node ); // Used by CircMatrix
    m_admitChanged = true;
    changed();
}

void eNode::stampSingAdm( ePin* epin, double admit )
{
    Connection* conn = m_firstSingAdm;
    while( conn ){
        if( conn->epin == epin ) { conn->value = admit; break; } // Connection found
        conn = conn->next;
    }
    /// if( admit == 0 ) m_switched = true;
    m_admitChanged = true;
    changed();
}

void eNode::createCurrent( ePin* epin )
{
    Connection* first = m_firstCurrent;

    while( first ){
        if( first->epin == epin ) return; // Element already in the list
        first = first->next;
    }
    Connection* conn = new Connection( epin );
    conn->next = m_firstCurrent;  // Prepend
    m_firstCurrent = conn;
}

void eNode::stampCurrent( ePin* epin, double current ) // Be sure msg doesn't come from this node
{
    Connection* conn = m_firstCurrent;
    while( conn ){
        if( conn->epin == epin ) { conn->value = current; break; } // Connection found
        conn = conn->next;
    }
    m_currChanged = true;
    changed();
}

void eNode::changed()
{
    if( m_changed ) return;
    m_changed = true;
    Simulator::self()->addToChangedNodes( this );
}

void eNode::stampMatrix()
{
    if( m_nodeNum == 0 ) return;
    m_changed = false;

    if( m_admitChanged )
    {
        m_totalAdmit = 0;
        //if( m_switched ) m_totalAdmit += 1e-12; // Weak connection to ground

        if( m_single ){
            Connection* conn = m_firstAdmit;
            while( conn ){ m_totalAdmit += conn->value; conn = conn->next; } // Calculate total admitance
        }else{
            Connection* na = m_nodeAdmit;
            while( na ){ na->value = 0; na = na->next; } // Clear nodeAdmit

            Connection* conn = m_firstAdmit; // Full Admitances
            while( conn ){
                double adm = conn->value;
                int  enode = conn->node;

                if( enode > 0 ){        // Calculate admitances to nodes
                    na = m_nodeAdmit;
                    while( na ){
                        if( na->node == enode ){ na->value += adm; break; }
                        na = na->next;
                    }
                }
                m_totalAdmit += adm;    // Calculate total admitance
                conn = conn->next;
            }
            CircMatrix::self()->stampDiagonal( m_nodeGroup, m_nodeNum, m_totalAdmit ); // Stamp diagonal

            conn = m_firstSingAdm;      // Single admitance values
            while( conn ){
                double adm = conn->value;
                int  enode = conn->node;

                if( enode > 0 ){        // Add sinle admitance to node
                    na = m_nodeAdmit;
                    while( na ){
                        if( na->node == enode ){ na->value += adm; break; }
                        na = na->next;
                    }
                }
                conn = conn->next;
            }
            na = m_nodeAdmit;
            while( na ){                  // Stamp non diagonal
                int    enode = na->node;
                double admit = na->value;
                if( enode > 0 ) CircMatrix::self()->stampMatrix( m_nodeNum, enode, -admit );
                na = na->next;
            }
        }
        m_admitChanged = false;
    }
    if( m_currChanged ){
        m_totalCurr  = 0;

        Connection* conn = m_firstCurrent;
        while( conn ){ m_totalCurr += conn->value; conn = conn->next; } // Calculate total current

        if( !m_single ) CircMatrix::self()->stampCoef(  m_nodeGroup, m_nodeNum, m_totalCurr );
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
    if( m_volt == v ) return;

    m_voltChanged = true; // Used for wire animation
    m_volt = v;

    CallBackElement* linked = m_voltChEl; // VoltChaneg callback
    while( linked )
    {
        eElement* el = linked->element;
        linked = linked->next;
        if( el->added ) continue;
        Simulator::self()->addToChangedList( el );
        el->added = true;
    }
    linked = m_nonLinEl ;              // Non Linear callback
    while( linked )
    {
        eElement* el = linked->element;
        linked = linked->next;
        if( el->added ) continue;
        Simulator::self()->addToNoLinList( el );
        el->added = true;
    }
}

void eNode::addEpin( ePin* epin )
{ if( !m_ePinList.contains(epin)) m_ePinList.append(epin); }

void eNode::remEpin( ePin* epin )
{
    if( m_ePinList.contains(epin) ) m_ePinList.removeOne( epin );
}

void eNode::clear()
{
    for( ePin* epin : m_ePinList ){
        epin->setEnode( nullptr );
        epin->setEnodeComp( nullptr );
}   }

QList<int> eNode::getConnections()
{ return m_nodeList; }

void eNode::voltChangedCallback( eElement* el )
{
    CallBackElement* changed = m_voltChEl;
    while( changed )
    {
        if( el == changed->element ) return; // Element already in the list
        changed = changed->next;
    }
    CallBackElement* newLinked = new CallBackElement( el );
    newLinked->next = m_voltChEl; // Prepend
    m_voltChEl = newLinked;
}

void eNode::remFromChangedCallback( eElement* el )
{
    CallBackElement* changed = m_voltChEl;
    CallBackElement* last  = nullptr;
    CallBackElement* next  = nullptr;

    while( changed ){
        next = changed->next;
        if( el == changed->element )
        {
            if( last ) last->next = next;
            else       m_voltChEl = next;
            delete changed;//changed->next = nullptr;
        }
        else last = changed;
        changed = next;
    }
}

void eNode::addToNoLinList( eElement* el )
{
    CallBackElement* changed = m_nonLinEl;
    while( changed )
    {
        if( el == changed->element ) return; // Element already in the list
        changed = changed->next;
    }
    CallBackElement* newLinked = new CallBackElement( el );
    newLinked->next = m_nonLinEl; // Prepend
    m_nonLinEl = newLinked;
    //qDebug() <<m_id<< el->getId();
}

void eNode::updateConnectors()
{
    if( !m_voltChanged ) return;
    m_voltChanged = false;

    for( ePin* epin : m_ePinList ){
        Pin* pin = epin->getPin();
        if( pin && pin->isVisible() ){
            Connector* conn = pin->connector();
            if( conn ) conn->updateLines();
        }
    }
}

void eNode::clearElmList( CallBackElement* first )
{
    while( first ){
        CallBackElement* del = first;
        first = first->next;
        delete del;
    }
}

void eNode::clearConnList( Connection* first )
{
    while( first ){
        Connection* del = first;
        first = first->next;
        delete del;
    }
}
