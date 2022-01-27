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

#include "diode.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "pin.h"

#include "doubleprop.h"
#include "stringprop.h"

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

Diode::Diode( QObject* parent, QString type, QString id, bool zener )
     : Comp2Pin( parent, type, id )
     , eDiode( id )
{
    m_area = QRect(-12, -8, 24, 16 );

    setEpin( 0, m_pin[0] );
    setEpin( 1, m_pin[1] );

    createSerRes();
    m_isZener = zener;
    if( zener ){
        m_diodeType = "diode";
        setModel( "Zener Default" );
    }else{
        m_diodeType = "zener";
        setModel( "Diode Default" );
    }
    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new StringProp<Diode>( "Model", tr("Model"),"", this, &Diode::model,  &Diode::setModel, "enum" ),
    }} );
    addPropGroup( { tr("Electric"), {
new DoubProp<Diode>( "Threshold" , tr("Forward Voltage"),"V", this, &Diode::threshold,  &Diode::setThreshold ),
new DoubProp<Diode>( "MaxCurrent", tr("Max Current")    ,"A", this, &Diode::maxCurrent, &Diode::setMaxCurrent ),
new DoubProp<Diode>( "Resistance", tr("Resistance")     ,"Ω", this, &Diode::res,        &Diode::setResSafe ),
    }} );
    addPropGroup( { tr("Advanced"), {
new DoubProp<Diode>( "BrkDownV"  , tr("Breakdown Voltage")   ,"V" , this, &Diode::brkDownV, &Diode::setBrkDownV ),
new DoubProp<Diode>( "SatCurrent", tr("Saturation Current")  ,"nA", this, &Diode::satCur,   &Diode::setSatCur ),
new DoubProp<Diode>( "EmCoef"    , tr("Emission Coefficient"),""  , this, &Diode::emCoef,   &Diode::setEmCoef ),
    }} );
}
Diode::~Diode(){}

bool Diode::setPropStr( QString prop, QString val )
{
    if( prop =="Zener_Volt" ) setZenerV( val.toDouble() ); //  Old: TODELETE
    else return Component::setPropStr( prop, val );
    return true;
}

void Diode::initialize()
{
    m_crashed = false;
    m_warning = false;
    eDiode::initialize();
    update();
}

void Diode::updateStep()
{
    if( m_current > m_maxCur ){
        m_warning = true;
        m_crashed = m_current > m_maxCur*2;
        update();
    }else{
        if( m_warning ) update();
        m_warning = false;
        m_crashed = false;
}   }

void Diode::setZenerV( double zenerV ) // Compatibility with old circuits.
{
    m_isZener = zenerV > 0;
    eDiode::setBrkDownV( zenerV );
}

void Diode::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::black );

 static const QPointF points[3] = {
        QPointF( 7, 0 ),
        QPointF(-8,-7 ),
        QPointF(-8, 7 )              };
    p->drawPolygon(points, 3);

    QPen pen = p->pen();
    pen.setWidth( 3 );
    p->setPen( pen );
    p->drawLine( 7, -6, 7, 6 );
   
    if( m_isZener ){
        p->drawLine( 7,-6, 4,-6 );
        p->drawLine( 7, 6, 10, 6 );
}  }
