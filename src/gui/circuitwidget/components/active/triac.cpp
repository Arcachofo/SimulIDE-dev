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
#include <QDebug>
#include <math.h>

#include "triac.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "e-diode.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"

Component* Triac::construct( QObject* parent, QString type, QString id )
{ return new Triac( parent, type, id ); }

LibraryItem* Triac::libraryItem()
{
    return new LibraryItem(
        tr( "Triac" ),
        tr( "Active" ),
        "triac.png",
        "Triac",
        Triac::construct );
}

Triac::Triac( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area =  QRectF( -8, -16, 16, 32 );
    setLabelPos(-12,-30, 0 );

    m_holdCurr = 0.0082;
    m_trigCurr = 0.01;
    m_gateRes  = 100;

    // Pin0--|--ePin0--diode1--ePin1--midEnode--ePin4--resistor--Pin1
    //       |--ePin2--diode2--ePin3--|
    // Pin2----------resistGa--ePin5--|
    m_pin.resize( 3 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), id+"-lPin", 0, this);
    m_pin[1] = new Pin( 0,   QPoint( 16, 0 ), id+"-rPin", 1, this);
    m_pin[2] = new Pin( 0,   QPoint( 16, 12 ), id+"-gPin", 2, this);
    m_pin[2]->setPinAngle(-26 );
    m_pin[2]->setLength( 10 );

    setNumEpins( 6 );

    m_diode1 = new eDiode( id+"-dio1" );
    m_diode1->setEpin( 0, m_ePin[0] );
    m_diode1->setEpin( 1, m_ePin[1] );
    m_diode1->setModel( "Diode Default" );

    m_diode2 = new eDiode( id+"-dio2" );
    m_diode2->setEpin( 0, m_ePin[2] );
    m_diode2->setEpin( 1, m_ePin[3] );
    m_diode2->setModel( "Diode Default" );

    m_resistor = new eResistor( m_elmId+"-resistor");
    m_resistor->setEpin( 0, m_ePin[4] );
    m_resistor->setEpin( 1, m_pin[1] );

    m_resistGa = new eResistor( m_elmId+"-gateRes");
    m_resistGa->setEpin( 0, m_pin[2] );
    m_resistGa->setEpin( 1, m_ePin[5] );

    addPropGroup( { tr("Main"), {
new DoubProp<Triac>( "GateRes" , tr("Gate Resistance"),"Ω", this, &Triac::gateRes , &Triac::setGateRes ),
new DoubProp<Triac>( "TrigCurr", tr("Trigger Current"),"A", this, &Triac::trigCurr, &Triac::setTrigCurr ),
new DoubProp<Triac>( "HoldCurr", tr("Holding Current"),"A", this, &Triac::holdCurr, &Triac::setHoldCurr )
    }} );
}
Triac::~Triac()
{
    delete m_diode1;
    delete m_diode2;
    delete m_resistor;
    delete m_resistGa;
}

void Triac::initialize()
{
    m_midEnode = new eNode( m_elmId+"-mideNode");
}

void Triac::stamp()
{
    m_state = false;

    eNode* node0 = m_pin[0]->getEnode();
    eNode* node1 = m_pin[1]->getEnode();
    eNode* node2 = m_pin[2]->getEnode();

    m_diode1->getEpin(0)->setEnode( node0 );
    m_diode1->getEpin(1)->setEnode( m_midEnode );

    m_diode2->getEpin(0)->setEnode( m_midEnode );
    m_diode2->getEpin(1)->setEnode( node0 );

    m_resistor->getEpin(0)->setEnode( m_midEnode );
    m_resistGa->getEpin(1)->setEnode( m_midEnode );

    if( node0 ) node0->addToNoLinList( this );
    if( node1 ) node1->addToNoLinList( this );
    if( node2 ) node2->addToNoLinList( this );

    m_resistor->setRes( 10e5 );
    m_resistGa->setRes( m_gateRes );
}

void Triac::updateStep()
{
    if( Circuit::self()->animate() ) update();
}

void Triac::voltChanged()
{
    double current1 = m_resistor->current();
    double currentG = m_resistGa->current();
    bool state = m_state;
//qDebug() << "Triac::voltChanged"<<current1 <<currentG;
    if( fabs(current1) < m_holdCurr ) state = false; /// Mingw needs fabs
    if( fabs(currentG) > m_trigCurr ) state = true;

    if( m_state != state )
    {
        m_state = state;
        double res = state ? .01 : 10e5;
        m_resistor->setRes( res );
    }
}

void Triac::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points1[3] = {
       QPointF(-7,-8  ),
       QPointF( 8, 0 ),
       QPointF( 8,-15 )          };
   p->drawPolygon( points1, 3 );

 static const QPointF points2[3] = {
        QPointF( 7, 8  ),
        QPointF(-8, 15 ),
        QPointF(-8, 0  )          };
    p->drawPolygon( points2, 3 );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-8,-16,-8, 16 );
    p->drawLine( 8,-16, 8, 16 );
}
