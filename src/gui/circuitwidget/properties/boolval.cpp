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

#include "boolval.h"
#include "component.h"
#include "propdialog.h"
#include "comproperty.h"

BoolVal::BoolVal( PropDialog* parent, Component* comp, ComProperty* prop )
       : PropVal( parent, comp, prop )
{
    setupUi(this);
}
BoolVal::~BoolVal() {;}

void BoolVal::setup()
{
    nameLabel->setText( m_property->capt() );
    m_blocked = true;

    bool checked = (m_property->getValStr() == "true");
    trueVal->setChecked( checked );
    m_blocked = false;

    this->adjustSize();
}

void BoolVal::on_trueVal_toggled( bool checked )
{
    if( m_blocked ) return;
    m_property->setValStr( checked ? "true" : "false" );
    m_propDialog->updtValues();
}

void BoolVal::updtValues()
{
    bool checked = (m_property->getValStr() == "true");
    trueVal->setChecked( checked );
}
