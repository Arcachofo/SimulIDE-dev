/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "buffer.h"
#include "itemlibrary.h"
#include "iopin.h"

#include "boolprop.h"

Component* Buffer::construct( QObject* parent, QString type, QString id )
{ return new Buffer( parent, type, id ); }

LibraryItem* Buffer::libraryItem()
{
    return new LibraryItem(
        tr( "Buffer" ),
        tr( "Logic/Gates" ),
        "buffer.png",
        "Buffer",
        Buffer::construct );
}

Buffer::Buffer( QObject* parent, QString type, QString id )
      : Gate( parent, type, id, 1 )
{
    m_area = QRect( -19, -17, 38, 34 ); 
    
    setOePin( new IoPin( 90, QPoint( 0,-12 ), m_id+"-ePin-outEnable", 0, this, input ) );
    setTristate( false );

    addPropGroup( { tr("Main"), {
new BoolProp<Buffer>( "Tristate"      , tr("Tristate")     ,"", this, &Buffer::tristate,   &Buffer::setTristate ),
new BoolProp<Buffer>( "Inverted"      , tr("Invert Output"),"", this, &Buffer::invertOuts, &Buffer::setInvertOuts ),
new BoolProp<Buffer>( "Open_Collector", tr("Open Drain")   ,"", this, &Buffer::openCol,    &Buffer::setOpenCol )
    }} );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()+IoComponent::outputProps() } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps() } );
}
Buffer::~Buffer(){}

void Buffer::setTristate( bool t )  // Activate or deactivate OE Pin
{
    LogicComponent::setTristate( t );
    m_oePin->setLabelText( "" );
}

QPainterPath Buffer::shape() const
{
    QPainterPath path;
    QVector<QPointF> points;
    
    points << QPointF(-9,-9 )
           << QPointF(-9, 9 )
           << QPointF( 9, 1 )
           << QPointF( 9,-1 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Buffer::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    static const QPointF points[4] = {
        QPointF(-8,-8 ),
        QPointF(-8, 8 ),
        QPointF( 8, 1 ),
        QPointF( 8,-1 )             };

    p->drawPolygon( points, 4 );
}
