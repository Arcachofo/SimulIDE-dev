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

#include "enumval.h"
#include "component.h"
#include "propdialog.h"
#include "comproperty.h"

EnumVal::EnumVal( PropDialog* parent, Component* comp, ComProperty* prop )
       : PropVal( parent, comp, prop )
{
    setupUi(this);
}
EnumVal::~EnumVal() {}

void EnumVal::setup()
{
    valLabel->setText( m_property->capt() );
    m_blocked = true;

    QStringList enumNames = m_component->getEnumNames( m_propName );/// enums.sort();
    for( QString val : enumNames ) valueBox->addItem( val );

    m_enums = m_component->getEnumUids( m_propName );

    QString valStr = m_property->getValStr();
    valueBox->setCurrentIndex( m_enums.indexOf( valStr) );

    m_blocked = false;
    updtValues();
    this->adjustSize();
}

void EnumVal::on_showVal_toggled( bool checked )
{
    if( m_blocked ) return;
    m_blocked = true;

    if( checked ){
        m_component->setShowProp( m_propName );
        m_component->setValLabelText( valueBox->currentText() );
    }
    else m_component->setShowProp( "" );
    m_propDialog->updtValues();
    m_blocked = false;
}

void EnumVal::on_valueBox_currentIndexChanged( QString val )
{
    if( m_blocked ) return;
    int index = valueBox->currentIndex();

    //if( m_property->type() == "string" )
        m_property->setValStr( m_enums.at( index ) );
    //else m_property->setValStr( QString::number( index ) );

    if( showVal->isChecked() ) m_component->setValLabelText( val );
}

void EnumVal::updtValues()
{
    if( m_blocked ) return;
    m_blocked = true;

    bool checked = m_component->showProp() == m_propName;
    showVal->setChecked( checked );

    m_blocked = false;
}

