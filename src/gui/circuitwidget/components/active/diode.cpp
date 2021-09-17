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

#include "diode.h"
#include "itemlibrary.h"
#include "connector.h"
#include "pin.h"

static const char* Diode_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Threshold"),
    QT_TRANSLATE_NOOP("App::Property","Zener Volt")
};

Component* Diode::construct( QObject* parent, QString type, QString id )
{ return new Diode( parent, type, id ); }

LibraryItem* Diode::libraryItem()
{
    return new LibraryItem(
            tr( "Diode" ),
            tr( "Active" ),
            "diode.png",
            "Diode",
            Diode::construct);
}

Diode::Diode( QObject* parent, QString type, QString id, bool zener)
     : Component( parent, type, id )
     , eDiode( id )
{
    Q_UNUSED( Diode_properties );

    m_area = QRect( -12, -8, 24, 16 );
    
    double brkDownV = zener ? 5.6 : 0;
    m_isZener = zener;

    m_pin.resize(2);
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), m_id+"-lPin", 0, this ); // pPin
    setEpin( 0, m_pin[0] );

    m_pin[1] = new Pin( 0, QPoint( 16, 0 ), m_id+"-rPin", 1, this ); // nPin
    setEpin( 1, m_pin[1] );

    createSerRes();
    SetParameters( 1.7143528192808883e-7, 2, brkDownV, 0 );
}
Diode::~Diode(){}

QList<propGroup_t> Diode::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Threshold", tr("Forward Voltage"),"V"} );
    mainGroup.propList.append( {"Resistance", tr("Resistance"),"Ω"} );

    propGroup_t advanced { tr("Advanced") };
    advanced.propList.append( {"BrkDownV", tr("Breakdown Voltage"),"V"} );
    advanced.propList.append( {"SatCur_nA", tr("Saturation Current"),"nA"} );
    advanced.propList.append( {"EmCoef", tr("Emission Coefficient"),""} );
    return {mainGroup, advanced};
}

void Diode::setZenerV( double zenerV ) // Compatibility with old circuits.
{
    m_isZener = zenerV > 0;
    eDiode::setBrkDownV( zenerV );
}

void Diode::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

    static const QPointF points[3] = {
        QPointF( 7, 0 ),
        QPointF(-8,-7 ),
        QPointF(-8, 7 )              };

   p->drawPolygon(points, 3);

   QPen pen = p->pen();
   pen.setWidth(3);
   p->setPen(pen);

   p->drawLine( 7, -6, 7, 6 );
   
   if( m_isZener )
   {
       p->drawLine( 7,-6, 4,-6 );
       p->drawLine( 7, 6, 10, 6 );
}  }

#include "moc_diode.cpp"
