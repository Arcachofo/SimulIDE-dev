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
#include <math.h>

#include "inductor.h"
#include "itemlibrary.h"
#include "pin.h"

#include "doubleprop.h"
#include "intprop.h"

Component* Inductor::construct( QObject* parent, QString type, QString id )
{ return new Inductor( parent, type, id ); }

LibraryItem* Inductor::libraryItem()
{
    return new LibraryItem(
        tr( "Inductor" ),
        tr( "Reactive" ),
        "inductor.png",
        "Inductor",
        Inductor::construct);
}

Inductor::Inductor( QObject* parent, QString type, QString id )
        : Reactive( parent, type, id )
{
    m_pin[0]->setLength( 4.5 );
    m_pin[1]->setLength( 4.5 );

    m_value = 1; // H

    addPropGroup( { tr("Main"), {
new DoubProp<Inductor>( "Inductance", tr("Inductance")     , "H"     , this, &Inductor::value,    &Inductor::setValue ),
new DoubProp<Inductor>( "Resistance", tr("Resistance")     , "Ω"     , this, &Inductor::resist  , &Inductor::setResist ),
new DoubProp<Inductor>( "InitVolt"  , tr("Initial Current"), "A"     , this, &Inductor::initCurr, &Inductor::setInitCurr ),
new IntProp <Inductor>( "AutoStep"  , tr("Auto Step")      , "_Steps", this, &Inductor::autoStep, &Inductor::setAutoStep, "uint" )
    } } );

    setShowProp("Inductance");
    setPropStr( "Inductance", "1" );
}
Inductor::~Inductor(){}

void Inductor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(2.8);
    p->setPen(pen);
    
    p->drawArc( QRectF(-12,-4.5, 10, 10 ),-45*16 , 220*16 );
    p->drawArc( QRectF( -5,-4.5, 10, 10 ), 225*16,-270*16 );
    p->drawArc( QRectF(  2,-4.5, 10, 10 ), 225*16,-220*16 );
}
