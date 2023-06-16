/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
    m_propDialog->changed();
    m_blocked = false;
}

void EnumVal::on_valueBox_currentIndexChanged( QString val )
{
    if( m_blocked ) return;
    int index = valueBox->currentIndex();
    m_property->setValStr( m_enums.at( index ) );

    if( showVal->isChecked() ) m_component->setValLabelText( val );
    m_propDialog->changed();
}

void EnumVal::updtValues()
{
    if( m_blocked ) return;
    m_blocked = true;

    bool checked = m_component->showProp() == m_propName;
    showVal->setChecked( checked );

    m_blocked = false;
}

