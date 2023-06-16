/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
        m_useMult = false;
        unitBox->setStyleSheet( "QComboBox::drop-down {border-width: 0px;} \
                                 QComboBox::down-arrow {image: url(noimg); border-width: 0px;}");
        if( unit.startsWith("_") ) unitBox->addItem( unit.remove("_") ); // No multiplier
    }else{
        m_useMult = true;
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
    m_propDialog->changed();
    m_blocked = false;
}

void NumVal::on_valueBox_valueChanged( double val )
{
    if( m_blocked ) return;
    m_blocked = true;

    if( m_useMult ) m_property->setValStr( getValWithUnit() );
    else            m_property->setValStr( QString::number( valueBox->value() ) );
    m_blocked = false;
    m_propDialog->updtValues();
    m_propDialog->changed();
}

void NumVal::on_unitBox_currentTextChanged( QString unit )
{
    if( m_blocked ) return;
    m_blocked = true;

    m_property->setValStr( getValWithUnit() );
    m_blocked = false;
    m_propDialog->updtValues();
    m_propDialog->changed();
}

void NumVal::updtValues()
{
    if( m_blocked ) return;
    m_blocked = true;

    showVal->setChecked( m_component->showProp() == m_propName );

    double multiplier = 1;
    if( m_useMult ) multiplier = getMultiplier( unitBox->currentText() );
    double val = m_property->getValue()/multiplier;
    valueBox->setValue( val );

    if( showVal->isChecked() ) m_component->setValLabelText( m_property->getValStr() );

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

