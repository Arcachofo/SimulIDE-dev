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

#include "doubleval.h"
#include "component.h"

DoubleVal::DoubleVal( QWidget* parent )
         : PropVal( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
    m_blocked = false;
    m_main    = false;
}
DoubleVal::~DoubleVal() {}

void DoubleVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void DoubleVal::setup( Component* comp, QString unit )
{
    m_blocked = true;
    m_component = comp;

    if( unit == "main" )
    {
        m_main = true;
        showVal->setChecked( comp->showVal() );

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
        showVal->setVisible( false );
        if( unit.isEmpty() ) unitBox->setVisible( false );
        else                 unitBox->addItem(" "+unit+" ");
        unitBox->setStyleSheet ("QComboBox::drop-down {border-width: 0px;} "
                 "QComboBox::down-arrow {image: url(noimg); border-width: 0px;}");
    }
    updtValues();
    m_blocked = false;
}

void DoubleVal::on_valueBox_valueChanged( double val )
{
    if( m_blocked ) return;
    m_blocked = true;
    m_component->setProperty( m_propName.toUtf8(), val );
    if( m_main ) setUnit();
    m_blocked = false;
}

void DoubleVal::on_showVal_toggled( bool checked )
{
    if( !m_main | m_blocked ) return;
    m_component->setShowVal( checked );
}

void DoubleVal::on_unitBox_currentTextChanged( QString unit )
{
    if( !m_main | m_blocked ) return;
    m_blocked = true;
    m_component->setUnit( unit );
    updtValues();
    m_blocked = false;
}

void DoubleVal::setUnit()
{
    QString unit = m_component->unit();
    //if( unit.size() > 2 ) unit = unit.right( 2 );
    unitBox->setCurrentText( unit );
}

void DoubleVal::updtValues()
{
    double val = m_component->property( m_propName.toUtf8() ).toDouble();
    valueBox->setValue( val );
    setUnit();
}
