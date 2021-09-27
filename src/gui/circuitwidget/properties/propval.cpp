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

#include <QComboBox>

#include "propval.h"
#include "propdialog.h"
#include "comproperty.h"

PropVal::PropVal( PropDialog* parent, Component* comp, ComProperty* prop )
       : QWidget( parent )
{
    m_propDialog = parent;
    m_component = comp;
    m_property  = prop;
    m_propName  = prop->name();
    m_blocked = false;
}
PropVal::~PropVal() {}

void PropVal::addDividers( QComboBox* unitBox, QString unit )
{
    unitBox->addItem("p"+unit );
    unitBox->addItem("n"+unit );
    unitBox->addItem("µ"+unit );
    unitBox->addItem("m"+unit );
}

void PropVal::addMultipliers( QComboBox* unitBox, QString unit )
{
    unitBox->addItem(    unit );
    unitBox->addItem("k"+unit );
    unitBox->addItem("M"+unit );
    unitBox->addItem("G"+unit );
    unitBox->addItem("T"+unit );
}

