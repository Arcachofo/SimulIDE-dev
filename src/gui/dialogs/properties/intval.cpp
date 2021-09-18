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

#include "intval.h"
#include "component.h"

IntVal::IntVal( QWidget* parent )
      : PropVal( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
    m_blocked = false;
}
IntVal::~IntVal() {}

void IntVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void IntVal::setup( Component* comp, QString unit )
{
    m_blocked = true;
    m_component = comp;

    if( unit == "main" )
    {
        //showVal->setChecked( comp->showVal() );
        unit = comp->unit().remove(" ");
        unitBox->addItem( "p"+unit);
        unitBox->addItem( "n"+unit);
        unitBox->addItem( "µ"+unit);
        unitBox->addItem( "m"+unit);
        unitBox->addItem( " "+unit);
        unitBox->addItem( "k"+unit);
        unitBox->addItem( "M"+unit);
        unitBox->addItem( "G"+unit);
        unitBox->addItem( "T"+unit);
    }else{
        //showVal->setVisible( false );
        QString un = unit;
        if( unit.size() < 3 ) un.prepend(" ");
        unitBox->addItem( un );
        unitBox->setStyleSheet ("QComboBox::drop-down {border-width: 0px;} "
                 "QComboBox::down-arrow {image: url(noimg); border-width: 0px;}");
    }
    updtValues();
    m_blocked = false;
    this->adjustSize();
}

void IntVal::on_value_valueChanged( int val )
{
    if( m_blocked ) return;
    m_component->setProperty( m_propName.toUtf8(), val );
    updtValues();
}

/*void IntVal::on_unitBox_currentTextChanged( QString unit )
{
    if( m_blocked ) return;
    m_blocked = true;
    m_component->setUnit( unit );
    updateValues();
    m_blocked = false;
}*/

void IntVal::updtValues()
{
    int val = m_component->property( m_propName.toUtf8() ).toInt();
    value->setValue( val );
}
