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
    m_color  = QColor( Qt::gray );
    m_area   = QRectF( -m_hSize/2, -m_vSize/2, m_hSize, m_vSize );
    setZValue( -1 );

    addPropGroup( { tr("Main"), {
new IntProp <Shape>( "H_size" , tr("Size X"),"_Pixels", this, &Shape::hSize,  &Shape::setHSize, "uint" ),
new IntProp <Shape>( "V_size" , tr("Size Y"),"_Pixels", this, &Shape::vSize,  &Shape::setVSize, "uint" ),
new IntProp <Shape>( "Border" , tr("Border"),"_Pixels", this, &Shape::border, &Shape::setBorder ),
new DoubProp<Shape>( "Z_Value", tr("Z Value"),""      , this, &Shape::zVal  , &Shape::setZVal )
    }} );
    addPropGroup( { tr("Color"), {
new StringProp<Shape>( "Color"  , tr("Color")  ,"", this, &Shape::colorStr, &Shape::setColorStr ),
new DoubProp  <Shape>( "Opacity", tr("Opacity"),"", this, &Shape::opac    ,  &Shape::setOpac )
    }} );
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
