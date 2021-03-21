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

BoolVal::BoolVal( QWidget* parent )
      : QWidget( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
}

void BoolVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    m_blocked = false;
    trueVal->setText( caption );
}

void BoolVal::setup( Component* comp )
{
    m_blocked = true;
    m_component = comp;
    bool checked = m_component->property( m_propName.toUtf8() ).toBool();
    trueVal->setChecked( checked );
    m_blocked = false;

    this->adjustSize();
}

void BoolVal::on_trueVal_toggled( bool checked )
{
    if( m_blocked ) return;
    m_component->setProperty( m_propName.toUtf8(), checked );
}


