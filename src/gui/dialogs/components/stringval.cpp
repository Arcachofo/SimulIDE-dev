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

StringVal::StringVal( QWidget* parent )
         : QWidget( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
}

void StringVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void StringVal::setup( Component* comp )
{
    m_component = comp;
    updatValue();
    this->adjustSize();
}

void StringVal::on_value_editingFinished()
{
    m_component->setProperty( m_propName.toUtf8(), value->text() );
    updatValue();
}

void StringVal::updatValue()
{
    QString text = m_component->property( m_propName.toUtf8() ).toString();
    value->setText( text );
}
