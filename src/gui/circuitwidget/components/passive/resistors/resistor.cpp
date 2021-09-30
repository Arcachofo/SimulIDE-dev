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

#include "resistor.h"
#include "itemlibrary.h"
#include "pin.h"

#include "doubleprop.h"

Component* Resistor::construct( QObject* parent, QString type, QString id )
{ return new Resistor( parent, type, id ); }

LibraryItem* Resistor::libraryItem()
{
    return new LibraryItem(
        tr( "Resistor" ),
        tr( "Resistors" ),
        "resistor.png",
        "Resistor",
        Resistor::construct);
}

Resistor::Resistor( QObject* parent, QString type, QString id )
        : Comp2Pin( parent, type, id )
        , eResistor( id )
{
    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];

    setShowProp("Resistance");

    addPropGroup( { tr("Main"), {
new DoubProp<Resistor>( "Resistance", tr("Resistance"), "Ω", this, &Resistor::getRes, &Resistor::setResSafe )
    } } );
}
Resistor::~Resistor(){}

void Resistor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );
}
