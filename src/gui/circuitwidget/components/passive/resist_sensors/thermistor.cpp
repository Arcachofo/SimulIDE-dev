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

#include <QPainter>
#include <math.h>

#include "thermistor.h"
#include "itemlibrary.h"

#include "intprop.h"
#include "propdialog.h"

Component* Thermistor::construct( QObject* parent, QString type, QString id )
{ return new Thermistor( parent, type, id ); }

LibraryItem* Thermistor::libraryItem()
{
    return new LibraryItem(
        QObject::tr( "Thermistor" ),
        QObject::tr( "Resistive Sensors" ),
        "thermistor.png",
        "Thermistor",
        Thermistor::construct);
}

Thermistor::Thermistor( QObject* parent, QString type, QString id )
          : ThermistorBase( parent, type, id  )
{
    m_bVal = 3455;
    m_r25  = 10000;

    addPropGroup( { QObject::tr("Parameters"), {
new IntProp<Thermistor>( "B"  , "B"  ,""  , this, &Thermistor::bVal, &Thermistor::setBval, "uint" ),
new IntProp<Thermistor>( "R25", "R25","Ω" , this, &Thermistor::r25,  &Thermistor::setR25,  "uint" )
    }} );
}
Thermistor::~Thermistor(){}

void Thermistor::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    double t0 = 25+273.15;      // Temp in Kelvin
    double t = m_value+273.15;
    double e = 2.7182;
    //double k = t*t0/(t-t0);
    //double res = m_r25/pow( e, m_bVal/k );
    double k = (t0-t)/(t*t0);
    double res = m_r25*pow( e, m_bVal*k );
    eResistor::setRes( res );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Temp" ) setValLabelText( getPropStr( "Temp" ) );
}

void Thermistor::setBval( int bval )
{
    m_bVal = bval;
    m_changed = true;
}

void Thermistor::setR25( int r25 )
{
    m_r25 = r25;
    m_changed = true;
}

void Thermistor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    p->drawRect( -10.5, -4, 21, 8 );
    
    p->drawLine(-8, 6,  6,-8 );
    p->drawLine( 6,-8, 10,-8 );
}
