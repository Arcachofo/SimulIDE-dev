/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shape.h"
#include "circuit.h"
#include "simulator.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"

#define tr(str) simulideTr("Shape",str)

Shape::Shape( QString type, QString id )
     : Component( type, id )
{
    m_graphical = true;
    
    m_hSize  = 50;
    m_vSize  = 30;
    m_border = 2;
    m_opac   = 1;
    m_color  = QColor( Qt::gray );
    m_area   = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    setZValue( -1 );

    m_changed = true;

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
        new IntProp<Shape>("H_size", tr("Width"), "_px"
                           , this, &Shape::hSize, &Shape::setHSize,0,"uint" ),

        new IntProp<Shape>("V_size", tr("Height"), "_px"
                           , this, &Shape::vSize, &Shape::setVSize,0,"uint" ),

        new DoubProp<Component>("Angle", tr("Angle"),"_º"
                           , this, &Component::getAngle,  &Component::setAngle ),

        new IntProp<Shape>("Border", tr("Border"), "_px"
                           , this, &Shape::border, &Shape::setBorder ),

        new DoubProp<Shape>("Z_Value", tr("Z Value"), ""
                           , this, &Shape::zVal, &Shape::setZVal )
    },0} );

    addPropGroup( { tr("Color"), {
        new StrProp<Shape>("Color", tr("Color"), ""
                          , this, &Shape::colorStr, &Shape::setColorStr,0,"color" ),

        new DoubProp<Shape>("Opacity", tr("Opacity"), ""
                           , this, &Shape::opac, &Shape::setOpac )
    },0} );
}
Shape::~Shape(){}

void Shape::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    update();

    m_area = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    Circuit::self()->update();
}

void Shape::setHSize( int size )
{
    if( size < 1 ) size = 1;
    m_hSize = size;
    changed();
}

void Shape::setVSize( int size )
{
    if( size < 1 ) size = 1;
    m_vSize = size;
    changed();
}

void Shape::setBorder( int border ) 
{ 
    if( border < 0 ) border = 0;
    m_border = border; 
    changed();
}

void Shape::setColor( QColor color )
{
    m_color = color;
    changed();
}

void Shape::setOpac( qreal op )
{
    if     ( op > 1 ) op = 1;
    else if( op < 0 ) op = 0;
    m_opac = op;
    changed();
}

void Shape::changed()
{
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}
