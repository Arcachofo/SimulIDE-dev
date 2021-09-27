/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "stringval.h"
#include "component.h"
#include "comproperty.h"

StringVal::StringVal( PropDialog* parent, Component* comp, ComProperty* prop )
         : PropVal( parent, comp, prop )
{
    setupUi(this);
}
StringVal::~StringVal() {}


void StringVal::setup()
{
    valLabel->setText( m_property->capt() );
    updatValue();
    this->adjustSize();
}

void StringVal::on_value_editingFinished()
{
    m_property->setValStr( value->text() );
    //m_component->setProperty( m_propName.toUtf8(), value->text() );
    //updatValue();
}

void StringVal::updatValue()
{
    //QString text = m_component->property( m_propName.toUtf8() ).toString();
    //value->setText( text );
}

void StringVal::updtValues()
{

}
