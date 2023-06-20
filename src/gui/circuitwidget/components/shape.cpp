/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "shape.h"
#include "circuit.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"

Shape::Shape( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
{
    m_graphical = true;
    
    m_hSize  = 50;
    m_vSize  = 30;
    m_border = 2;
    m_opac   = 1;
    m_color  = QColor( Qt::gray );
    m_area   = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    setZValue( -1 );

    addPropGroup( { tr("Main"), {
new IntProp <Shape>( "H_size" , tr("Size X"),"_Pixels", this, &Shape::hSize,  &Shape::setHSize,0,"uint" ),
new IntProp <Shape>( "V_size" , tr("Size Y"),"_Pixels", this, &Shape::vSize,  &Shape::setVSize,0,"uint" ),
new IntProp <Shape>( "Border" , tr("Border"),"_Pixels", this, &Shape::border, &Shape::setBorder ),
new DoubProp<Shape>( "Z_Value", tr("Z Value"),""      , this, &Shape::zVal  , &Shape::setZVal )
    },0} );
    addPropGroup( { tr("Color"), {
new StrProp<Shape>( "Color"  , tr("Color")  ,"", this, &Shape::colorStr, &Shape::setColorStr ),
new DoubProp  <Shape>( "Opacity", tr("Opacity"),"", this, &Shape::opac    ,  &Shape::setOpac )
    },0} );
}
Shape::~Shape(){}

void Shape::setHSize( int size )
{
    m_hSize = size;
    m_area = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    Circuit::self()->update();
}

void Shape::setVSize( int size )
{
    m_vSize = size;
    m_area = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    Circuit::self()->update();
}

void Shape::setBorder( int border ) 
{ 
    if( border < 0 ) border = 0;
    m_border = border; 
    update();
}

void Shape::setColor( QColor color )
{
    m_color = color;
    update();
}

void Shape::setOpac( qreal op )
{
    if     ( op > 1 ) op = 1;
    else if( op < 0 ) op = 0;
    m_opac = op;
    update();
}

