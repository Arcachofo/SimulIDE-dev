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

#include "varresistor.h"
#include "itemlibrary.h"
#include "simulator.h"

#include "doubleprop.h"
#include "propdialog.h"

Component* VarResistor::construct( QObject* parent, QString type, QString id )
{ return new VarResistor( parent, type, id ); }

LibraryItem* VarResistor::libraryItem()
{
    return new LibraryItem(
        tr( "Variable Resistor" ),
        tr( "Resistors" ),
        "varresistor.png",
        "VarResistor",
        VarResistor::construct);
}

VarResistor::VarResistor( QObject* parent, QString type, QString id )
           : VarResBase( parent, type, id  )
{
    setVal( 0 );

    addPropGroup( { tr("Main"), {
new DoubProp<VarResistor>( "Min_Resistance", tr("Minimum Resistance"), "Ω", this, &VarResistor::minVal, &VarResistor::setMinVal ),
new DoubProp<VarResistor>( "Max_Resistance", tr("Maximum Resistance"), "Ω", this, &VarResistor::maxVal, &VarResistor::setMaxVal ),
new DoubProp<VarResistor>( "Value_Ohm"     , tr("Current Value")     , "Ω", this, &VarResistor::getVal, &VarResistor::setVal ),
new DoubProp<VarResistor>( "Dial_Step"     , tr("Dial Step")         , "Ω", this, &VarResistor::getStep,&VarResistor::setStep ),
    } } );
}
VarResistor::~VarResistor(){}

void VarResistor::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    eResistor::setRes( m_value );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Value_Ohm" ) setValLabelText( getPropStr( "Value_Ohm" ) );
}

void VarResistor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;
    Component::paint( p, option, widget );

    p->drawRect( -10.5, -4, 21, 8 );
    p->drawLine(-6, 6, 8,-8 );
    p->drawLine( 8,-6, 8,-8 );
    p->drawLine( 8,-8, 6,-8 );
}
