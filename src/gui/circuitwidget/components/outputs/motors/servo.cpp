/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include "servo.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"
#include "e-node.h"

#include "doubleprop.h"

Component* Servo::construct( QObject* parent, QString type, QString id )
{ return new Servo( parent, type, id ); }

LibraryItem* Servo::libraryItem()
{
    return new LibraryItem(
        tr( "Servo" ),
        tr( "Motors" ),
        "servo.png",
        "Servo",
        Servo::construct );
}

Servo::Servo( QObject* parent, QString type, QString id )
     : LogicComponent( parent, type, id )
{
    m_graphical = true;
    m_width  = 10;
    m_height = 6;
    m_pos = 90;
    m_speed = 0.2;
    
    init({         // Inputs:
            "IL01 V+",
            "IL03 Gnd",
            "IL05 Sig"
        });

    for( int i=0; i<3;i++ ) m_inPin[i]->setLabelColor( QColor( 250, 250, 200 ) );

    m_clkPin = m_inPin[2];         // Input Clock

    setLabelPos(-16,-40, 0);
    setShowId( true );
    initialize();
    m_area = QRect( -40, -40, 96, 80 );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new DoubProp<Servo>( "Speed", tr("Speed "),tr("_sec/60º"), this, &Servo::speed, &Servo::setSpeed )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps() } );
//    addPropGroup( { tr("Edges")   , IoComponent::edgeProps() } );
}
Servo::~Servo(){}

void Servo::stamp()
{
    if( m_inPin[0]->isConnected()
      & m_inPin[1]->isConnected()
      & m_inPin[2]->isConnected() )
    {
        m_inPin[0]->changeCallBack( this, true );
        m_inPin[1]->changeCallBack( this, true );

        LogicComponent::stamp();
}   }

void Servo::initialize()
{
    m_targetPos = 90;
    m_pulseStart = 0;
    m_lastUpdate = Simulator::self()->circTime()/1e6;
    
    LogicComponent::initialize();
}

void Servo::updateStep()
{
    uint64_t step = Simulator::self()->circTime()/1e6;

    if( m_targetPos != m_pos )
    {
        double updateTime = (step - m_lastUpdate)/1e6;

        int maxMove  = updateTime/m_speed*60; // Maximum to move since last update
        int deltaPos = m_targetPos - m_pos;
        int absDeltaPos = abs(deltaPos);

        if( absDeltaPos > maxMove )
            deltaPos = absDeltaPos/deltaPos*maxMove;      // keep sign of deltaPos
        m_pos += deltaPos;
        Circuit::self()->update();
    }
    m_lastUpdate = step;
    update();
}

void Servo::voltChanged()
{
    updateClock();

    int time_us = Simulator::self()->circTime()/1e6;
    
    if(!(m_inPin[0]->getInpState()-m_inPin[1]->getInpState()))// not power
    {
        m_targetPos = 90;
        m_pulseStart = 0;
    }
    else if( m_clkState == Clock_Rising )
    {
        m_pulseStart = time_us;
    }
    else if( m_clkState == Clock_Falling )
    {
        if( m_pulseStart == 0 ) return;
        
        int steps = time_us - m_pulseStart;
        m_targetPos = (steps-1000)*180/1000;         // Map 1mS-2mS to 0-180ª

        if     ( m_targetPos>180 ) m_targetPos = 180;
        else if( m_targetPos<0 )   m_targetPos = 0;
        
        m_pulseStart = 0;
}   }

QPainterPath Servo::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    points << QPointF(-40,-24 )
           << QPointF(-40, 24 )
           << QPointF(  6, 24 )
           << QPointF(  6, 40 )
           << QPointF( 33, 40 )
           << QPointF( 57, 16 )
           << QPointF( 57,-16 )
           << QPointF( 33,-40 )
           << QPointF(  6,-40 )
           << QPointF(  6,-24 )
           << QPointF(-40,-24 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Servo::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    //p->drawPath( shape() );
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( QRect( -(m_width/2)*8,-(m_height/2)*8, m_width*8, m_height*8 ), 4, 4 );

    QPen pen = p->pen();
    pen.setColor( Qt::white);;
    p->setPen(pen);
    p->drawEllipse( 0,-16, 32, 32 );

    pen.setColor( Qt::black);;
    p->setPen(pen);
    p->setBrush( QColor( 255, 255, 255) );
    p->save();
    p->translate( 16, 0);
    p->rotate( m_pos-90 );
    p->drawRoundedRect( -8, -8, 48, 16, 8, 8 );
    p->restore();
}
