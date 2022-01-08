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

#include <QPainter>

#include "gate_or.h"
#include "itemlibrary.h"

#include "intprop.h"
#include "boolprop.h"

Component* OrGate::construct( QObject* parent, QString type, QString id )
{ return new OrGate( parent, type, id ); }

LibraryItem* OrGate::libraryItem()
{
    return new LibraryItem(
        tr( "Or Gate" ),
        tr( "Logic/Gates" ),
        "orgate.png",
        "Or Gate",
        OrGate::construct );
}

OrGate::OrGate( QObject* parent, QString type, QString id )
      : Gate( parent, type, id, 2 )
{
    addPropGroup( { tr("Main"), {
new IntProp <OrGate>( "Num_Inputs"    , tr("Input Size")   ,"_Inputs", this, &OrGate::numInps,    &OrGate::setNumInps, "uint" ),
new BoolProp<OrGate>( "Inverted"      , tr("Invert Output"),""       , this, &OrGate::invertOuts, &OrGate::setInvertOuts ),
new BoolProp<OrGate>( "Open_Collector", tr("Open Drain")   ,""       , this, &OrGate::openCol,    &OrGate::setOpenCol )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges")   , Gate::edgeProps() } );
}
OrGate::~OrGate(){}

bool OrGate::calcOutput( int inputs ) { return (inputs > 0); }

QPainterPath OrGate::shape() const
{
    QPainterPath path;
    QVector<QPointF> points;
    
    int size = (int)m_inPin.size()*4;
    
    points << QPointF(-14,-size-1 )
           << QPointF(-10,-size+4 )
           << QPointF( -9, 0 )
           << QPointF(-10, size-4  )
           << QPointF(-14, size+1 )
           << QPointF(  0, size )
           << QPointF( 10, 8  )
           << QPointF( 10,-8  )
           << QPointF(  0,-size );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void OrGate::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    int y_orig = m_area.y();
    int height = m_area.height();

    // Paint white background of gate
    Component::paint( p, option, widget );
    QPen pen = p->pen();
    p->setPen( Qt::NoPen );

    QPainterPath path;
    path.moveTo( -8, 0 );
    path.arcTo( -33, y_orig, 42, height, -90, 180 );
    path.moveTo( -8, 0 );
    path.arcTo( -16, y_orig, 8, height, -90, 180 );
    p->drawPath( path );

    // Draw curves
    p->setPen( pen );
    p->setBrush( Qt::NoBrush );
    // Output side arc
    p->drawArc( -28, y_orig, 37, height, -1520/*-16*95*/, 3040/*16*190*/ );
    // Input side arc
    p->drawArc( -16, y_orig, 8, height, -1440/*-16*90*/, 2880/*16*180*/ );

    /*QPen pen2( Qt::red, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen2 );
    p->drawPath( shape() );

    QPen pen3( Qt::blue, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen3 );
    p->drawRect( m_area );*/
}
