/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include <QPainter>

#include <math.h>

#include "scr.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "e-diode.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"

Component* SCR::construct( QObject* parent, QString type, QString id )
{ return new SCR( parent, type, id ); }

LibraryItem* SCR::libraryItem()
{
    return new LibraryItem(
        tr( "SCR" ),
        tr( "Rectifiers" ),
        "scr.png",
        "SCR",
        SCR::construct );
}

SCR::SCR( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area =  QRectF( -8, -16, 16, 32 );
    setLabelPos(-12,-30, 0 );

    m_holdCurr = 0.0082;
    m_trigCurr = 0.01;
    m_gateRes  = 100;

    m_pin.resize( 3 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);
    m_pin[2] = new Pin( 0,   QPoint( 16, 8 ), id+"-gPin", 2, this);
    m_pin[2]->setPinAngle(-40 );
    m_pin[2]->setLength( 12 );

    setNumEpins( 4 );

    // Pin0--resistorA--ePin0--midEnode--ePin1--diode--ePin2--|--ePin3--resistorC--Pin1
    // Pin2---------------------------------------------------|
    m_diode = new eDiode( id+"-dio1" );
    m_diode->setEpin( 0, m_ePin[1] );
    m_diode->setEpin( 1, m_ePin[2] );
    m_diode->setModel( "Diode Default" );

    m_resistorA = new eResistor( m_elmId+"-resistorA");
    m_resistorA->setEpin( 0, m_pin[0] );
    m_resistorA->setEpin( 1, m_ePin[0] );

    m_resistorC = new eResistor( m_elmId+"-resistorC");
    m_resistorC->setEpin( 0, m_ePin[3] );
    m_resistorC->setEpin( 1, m_pin[1] );

    addPropGroup( { tr("Main"), {
new DoubProp<SCR>( "GateRes" , tr("Gate Resistance"),"Ω", this, &SCR::gateRes , &SCR::setGateRes ),
new DoubProp<SCR>( "TrigCurr", tr("Trigger Current"),"A", this, &SCR::trigCurr, &SCR::setTrigCurr ),
new DoubProp<SCR>( "HoldCurr", tr("Holding Current"),"A", this, &SCR::holdCurr, &SCR::setHoldCurr )
    }} );
}
SCR::~SCR()
{
    delete m_diode;
    delete m_resistorA;
    delete m_resistorC;
}

void SCR::initialize()
{
    m_midEnode = new eNode( m_elmId+"-mideNode");
}

void SCR::stamp()
{
    // Pin0--resistorA--ePin0--midEnode--ePin1--diode--ePin2--|--ePin3--resistorC--Pin1
    // Pin2---------------------------------------------------|
    m_state = false;

    eNode* node0 = m_pin[0]->getEnode();
    eNode* node1 = m_pin[1]->getEnode();
    eNode* node2 = m_pin[2]->getEnode();

    m_diode->getEpin(0)->setEnode( m_midEnode );
    m_diode->getEpin(1)->setEnode( node2 );

    m_resistorA->getEpin(1)->setEnode( m_midEnode );
    m_resistorC->getEpin(0)->setEnode( node2 );

    if( node0 ) node0->addToNoLinList( this );
    if( node1 ) node1->addToNoLinList( this );
    if( node2 ) node2->addToNoLinList( this );

    m_resistorA->setRes( 10e5 );
    m_resistorC->setRes( m_gateRes );
}

void SCR::voltChanged()
{
    double currentA = m_resistorA->current();
    double currentC = -m_resistorC->current();
    bool state = m_state;
//qDebug() << "SCR::voltChanged"<<currentA <<currentC;

    double icmult = 1/m_trigCurr;
    double iamult = 1/m_holdCurr - icmult;

    state = -icmult*currentC + iamult*currentA > 1;

    if( m_state != state )
    {
        m_state = state;
        double res = state ? 0.0105 : 10e5;
        m_resistorA->setRes( res );
    }
}

void SCR::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points[3] = {
        QPointF( 7, 0 ),
        QPointF(-8,-7 ),
        QPointF(-8, 7 )              };
    p->drawPolygon(points, 3);

    QPen pen = p->pen();
    pen.setWidth( 3 );
    p->setPen( pen );
    p->drawLine( 7, -6, 7, 6 );
}
