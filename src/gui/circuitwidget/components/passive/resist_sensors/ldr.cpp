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
#include<math.h>

#include "ldr.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "intprop.h"
#include "propdialog.h"

Component* Ldr::construct( QObject* parent, QString type, QString id )
{ return new Ldr( parent, type, id ); }

LibraryItem* Ldr::libraryItem()
{
    return new LibraryItem(
        tr( "LDR" ),
        tr( "Resistive Sensors" ),
        "ldr.png",
        "LDR",
        Ldr::construct);
}

Ldr::Ldr( QObject* parent, QString type, QString id )
   : VarResBase( parent, type, id  )
{
    m_r1    = 127410;
    m_gamma = 0.8582;

    setVal( 1 );

    addPropGroup( { tr("Main"), {
new DoubProp<Ldr>( "Lux"      , tr("Current Value"),"Lux", this, &Ldr::getVal,  &Ldr::setVal ),
new DoubProp<Ldr>( "Min_Lux"  , tr("Minimum Value"),"Lux", this, &Ldr::minVal,  &Ldr::setMinVal ),
new DoubProp<Ldr>( "Max_Lux"  , tr("Maximum Value"),"Lux", this, &Ldr::maxVal,  &Ldr::setMaxVal ),
new DoubProp<Ldr>( "Dial_Step", tr("Dial Step")    ,"Lux", this, &Ldr::getStep, &Ldr::setStep )
    }} );
    addPropGroup( { tr("Parameters"), {
new DoubProp<Ldr>( "Gamma"    , tr("Gamma")        ,""   , this, &Ldr::gamma,   &Ldr::setGamma ),
new IntProp <Ldr>( "R1"       , tr("R1")           ,"Ω"  , this, &Ldr::r1,      &Ldr::setR1, "uint" )
    }} );
}
Ldr::~Ldr(){}

void Ldr::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    double res = double(m_r1)*pow( m_value, -m_gamma );
    eResistor::setRes( res );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Lux" ) setValLabelText( getPropStr( "Lux" ) );
}

void Ldr::setR1( int r1 )
{
    m_r1 = r1;
    m_changed = true;
}

void Ldr::setGamma( double ga )
{
    m_gamma = ga;
    m_changed = true;
}

void Ldr::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    p->drawRect( -10.5, -4, 21, 8 );
    
    p->drawLine(-5,-11,-1,-7 );
    p->drawLine(-1, -7,-1,-9 );
    p->drawLine(-1, -7,-3,-7 );

    p->drawLine( 1,-11, 5,-7 );
    p->drawLine( 5, -7, 5,-9 );
    p->drawLine( 5, -7, 3,-7 );
}
