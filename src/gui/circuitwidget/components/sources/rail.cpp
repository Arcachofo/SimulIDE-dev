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

#include "rail.h"
#include "simulator.h"
#include "iopin.h"
#include "itemlibrary.h"
#include "pin.h"

Component* Rail::construct( QObject* parent, QString type, QString id )
{ return new Rail( parent, type, id ); }

LibraryItem* Rail::libraryItem()
{
    return new LibraryItem(
        tr( "Rail." ),
        tr( "Sources" ),
        "voltage.png",
        "Rail",
        Rail::construct );
}

Rail::Rail( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
{
    setLabelPos(-64,-24 );

    m_area = QRect( -10, -10, 20, 20 );
    m_changed = false;

    m_pin.resize(1);
    m_out = new IoPin( 0, QPoint(16,0), id+"-outnod", 0, this, source );
    m_pin[0] = m_out;

    m_unit = "V";
    setVolt(5.0);
    setValLabelPos(-16, 8 , 0 ); // x, y, rot 
    setShowVal( true );
    
    setLabelPos(-16,-24, 0);
}
Rail::~Rail() { delete m_out; }

QList<propGroup_t> Rail::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Voltage", tr("Voltage"),"main"} );
    return {mainGroup};
}

double Rail::volt()
{
    return m_value;
}

void Rail::setVolt( double v )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();

    Component::setValue( v );       // Takes care about units multiplier
    updateOutput();

    if( pauseSim ) Simulator::self()->resumeSim();
}

void Rail::setUnit( QString un ) 
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();

    Component::setUnit( un );
    updateOutput();

    if( pauseSim ) Simulator::self()->resumeSim();
}

void Rail::updateOutput()
{
    m_out->setOutHighV( m_value*m_unitMult );
    m_out->setOutState( true );
}

void Rail::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->setBrush( QColor( 255, 166, 0 ) );

    p->drawRoundedRect( QRectF( -8, -8, 16, 16 ), 2, 2);
}

#include "moc_rail.cpp"

