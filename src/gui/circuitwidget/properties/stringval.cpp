/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "stringval.h"
#include "component.h"
#include "comproperty.h"

StringVal::StringVal( PropDialog* parent, Component* comp, ComProperty* prop )
         : PropVal( parent, comp, prop )
{
    setupUi(this);
}
StringVal::~StringVal() {}

void StringVal::setup()
{
    valLabel->setText( m_property->capt() );
    updtValues();
    this->adjustSize();
}

void StringVal::on_value_editingFinished()
{
    m_property->setValStr( value->text() );
    updtValues();
}

void StringVal::updtValues()
{
    QString text = m_property->getValStr();
    value->setText( text );
}
