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

#include "numval.h"
#include "component.h"
#include "propdialog.h"
#include "utils.h"
#include "comproperty.h"

NumVal::NumVal( PropDialog* parent, Component* comp, ComProperty* prop )
      : PropVal( parent, comp, prop )
{
    setupUi(this);
    m_type = prop->type();
    if     ( m_type == "double" ) return;
    else if( m_type == "uint" )   valueBox->setMinimum( 0 );
    valueBox->setDecimals( 0 );
}
NumVal::~NumVal() {}

void NumVal::setup()
{
    valLabel->setText( m_property->capt() );
    m_blocked = true;

    QString unit = m_property->unit();

    if( unit.isEmpty() || unit.startsWith("_") )
    {
        unitBox->setStyleSheet( "QComboBox::drop-down {border-width: 0px;} \
                                 QComboBox::down-arrow {image: url(noimg); border-width: 0px;}");
        if( unit.startsWith("_") )unitBox->addItem( unit.remove("_") );
    }else{
        QString un = unit;
        QRegExp r = QRegExp("^([pnµumkMGT])");
        if( r.indexIn( unit ) == 0 ) un.remove( 0, 1 ); // Remove multiplier

        if( m_type == "double") addDividers( unitBox, un );
        addMultipliers( unitBox, un );

        unitBox->setCurrentText( unit );
    }
    m_blocked = false;
    updtValues();
    ///this->adjustSize();
}

void NumVal::on_showVal_toggled( bool checked )
{
    if( m_blocked ) return;
    m_blocked = true;

    if( checked ){
        m_component->setShowProp( m_propName );
        m_component->setValLabelText( getValWithUnit() );
    }
    else m_component->setShowProp( "" );
    m_propDialog->updtValues();
    m_blocked = false;
}

void NumVal::on_valueBox_valueChanged( double val )
{
    if( m_blocked ) return;
    m_blocked = true;

    m_property->setValStr( getValWithUnit() );
    m_blocked = false;
    m_propDialog->updtValues();
}

void NumVal::on_unitBox_currentTextChanged( QString unit )
{
    if( m_blocked ) return;
    m_blocked = true;

    m_property->setValStr( getValWithUnit() );
    m_blocked = false;
    m_propDialog->updtValues();
}

void NumVal::updtValues()
{
    if( m_blocked ) return;
    m_blocked = true;

    showVal->setChecked( m_component->showProp() == m_propName );

    double multiplier = getMultiplier( unitBox->currentText() );
    double val = m_property->getValue()/multiplier;
    valueBox->setValue( val );

    /*QString valStr = m_property->getValStr();
    QStringList l = valStr.split(" ");
    valueBox->setValue( l.first().toDouble() );
    unitBox->setCurrentText( l.last() );*/

    m_blocked = false;
}

QString NumVal::getValWithUnit()
{
    QString valStr = QString::number( valueBox->value() );
    QString unit = unitBox->currentText();
    if( !unit.isEmpty() ) valStr.append(" "+unit );
    return valStr;
}

