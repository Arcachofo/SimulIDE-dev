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

#include <QPainter>
#include <QtMath>

#include "relay_base.h"
#include "simulator.h"
#include "circuit.h"
#include "e-node.h"

RelayBase::RelayBase( QObject* parent, QString type, QString id )
         : MechContact( parent, type, id )
{
    // This represents a coil, so is an Inductor in series with a Resistor: -Ind-Nod-Res-
    // We need to create the resistor, internal eNode and do the connections.

    m_ePin.resize(4);
    m_pin.resize(2);

    m_pin0 = 4;

    m_pin[0] = new Pin( 180, QPoint(-16-8,0), m_id+"-lPin", 0, this); // External Left pin to inductor
    m_pin[0]->setLength(4.5);
    m_pin[0]->setPos(-16, 0 );
    m_ePin[0] = m_pin[0];

    m_pin[1] = new Pin( 0, QPoint(16+8,0), m_id+"-rPin", 1, this); // External Right pin to resistor;
    m_pin[1]->setLength(4.5);
    m_pin[1]->setPos( 16, 0 );
    m_ePin[3] = m_pin[1];

    m_ePin[1] = new ePin( m_id+"-lePin", 1 ); // Internal Left pin to inductor
    m_ePin[2] = new ePin( m_id+"-rePin", 2 ); // Internal right pin to resistor

    m_resistor = new eResistor( m_id+"-resistor" );
    m_resistor->setRes( 100 );
    m_resistor->setEpin( 0, m_ePin[2] );
    m_resistor->setEpin( 1, m_ePin[3] );

    m_inductor = new eInductor( m_id+"-inductor" );
    m_inductor->setInd( 0.1 );  // 100 mH
    m_inductor->setEpin( 0, m_ePin[0] );
    m_inductor->setEpin( 1, m_ePin[1] );

    m_trigCurrent = 0.02;
    m_relCurrent  = 0.01;

    setValLabelPos(-16, 6, 0);
    setLabelPos(-16, 8, 0);
    //setShowVal( true );

    SetupSwitches( 1, 1 );
}
RelayBase::~RelayBase(){}

void RelayBase::initialize()
{
    m_relayOn = false;

    if( Simulator::self()->isRunning() )
        m_internalEnode = new eNode( m_id+"-internaleNode" );
}

void RelayBase::stamp()
{
    m_ePin[1]->setEnode( m_internalEnode );
    m_ePin[2]->setEnode( m_internalEnode );

    MechContact::stamp();

    m_pin[0]->changeCallBack( this );
    m_ePin[1]->changeCallBack( this );
}

void RelayBase::voltChanged()
{
    double indCurr = qFabs( m_inductor->indCurrent() );
    bool relayOn;

    if( m_relayOn ) relayOn = ( indCurr > m_relCurrent );
    else            relayOn = ( indCurr > m_trigCurrent );
    m_relayOn = relayOn;

    if( m_nClose ) relayOn = !relayOn;
    if( relayOn != m_closed ) setSwitch( relayOn );
}

void RelayBase::remove()
{
    delete m_resistor;
    delete m_inductor;

    MechContact::remove();
}

void RelayBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush(Qt::white);
    p->drawRect( m_area );

    QPen pen = p->pen();                                       // Draw Coil
    pen.setWidth(2.8);
    p->setPen(pen);

    QRectF rectangle(-12,-4.5, 10, 10 );
    int startAngle = -45 * 16;
    int spanAngle = 220 * 16;
    p->drawArc(rectangle, startAngle, spanAngle);

    QRectF rectangle2(-5,-4.5, 10, 10 );
    startAngle = 225 * 16;
    spanAngle = -270 * 16;
    p->drawArc(rectangle2, startAngle, spanAngle);

    QRectF rectangle3(2,-4.5, 10, 10 );
    startAngle = 225 * 16;
    spanAngle = -220 * 16;
    p->drawArc(rectangle3, startAngle, spanAngle);

    MechContact::paint( p, option, widget );
}
