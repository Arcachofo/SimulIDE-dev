/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "battery.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "pin.h"

#include "doubleprop.h"

Component* Battery::construct( QObject* parent, QString type, QString id )
{ return new Battery( parent, type, id ); }

LibraryItem* Battery::libraryItem()
{
    return new LibraryItem(
        tr( "Battery" ),
        tr( "Sources" ),
        "battery.png",
        "Battery",
        Battery::construct );
}

Battery::Battery( QObject* parent, QString type, QString id )
      : Comp2Pin( parent, type, id )
      , eElement( id )
{
    m_area = QRect( -10, -10, 20, 20 );

    //m_unit = "V";
    m_volt = 5;
    //setShowVal( true );

    setLabelPos(-18,-22, 0 );
    setValLabelPos(-10, 10, 0 ); // x, y, rot

    addPropGroup( { tr("Main"), {
new DoubProp<Battery>( "Voltage", tr("Voltage"), "V", this, &Battery::volt, &Battery::setVolt )
    } } );
}
Battery::~Battery() {}

void Battery::stamp()
{
    m_pin[0]->setEnodeComp( m_pin[1]->getEnode() );
    m_pin[1]->setEnodeComp( m_pin[0]->getEnode() );
    m_pin[0]->stampAdmitance( 1/cero_doub );
    m_pin[1]->stampAdmitance( 1/cero_doub );
    m_pin[0]->stampCurrent( m_volt/cero_doub );
    m_pin[1]->stampCurrent(-m_volt/cero_doub );
}

void Battery::setVolt( double volt )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    if( volt < 1e-12 ) volt = 1e-12;
    m_volt = volt;
}

void Battery::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-7,-8,-7, 8 );
    p->drawLine(-2,-3,-2, 3 );
    p->drawLine( 3,-8, 3, 8 );
    p->drawLine( 8,-3, 8, 3 );
}
