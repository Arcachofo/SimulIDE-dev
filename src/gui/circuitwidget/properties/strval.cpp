/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "strval.h"
#include "component.h"
#include "comproperty.h"
#include "propdialog.h"

StrVal::StrVal( PropDialog* parent, CompBase* comp, ComProperty* prop )
      : PropVal( parent, comp, prop )
{
    setupUi(this);
}
StrVal::~StrVal() {}

void StrVal::setup( bool )
{
    valLabel->setText( m_property->capt() );
    updtValues();
    this->adjustSize();
}

void StrVal::on_value_editingFinished()
{
    m_property->setValStr( value->text() );
    updtValues();
    m_propDialog->changed();
}

void StrVal::updtValues()
{
    QString text = m_property->getValStr();
    value->setText( text );
}
