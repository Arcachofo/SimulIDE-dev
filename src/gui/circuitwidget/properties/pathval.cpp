/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "pathval.h"
#include "component.h"
#include "comproperty.h"
#include "propdialog.h"
#include "utils.h"

PathVal::PathVal( PropDialog* parent, CompBase* comp, ComProperty* prop )
       : PropVal( parent, comp, prop )
{
    setupUi(this);
}
PathVal::~PathVal() {}

void PathVal::setup( bool )
{
    valLabel->setText( m_property->capt() );
    updtValues();
    this->adjustSize();
}

void PathVal::on_value_editingFinished()
{
    m_property->setValStr( value->text() );
    updtValues();
    m_propDialog->changed();
}

void PathVal::on_setPathButton_clicked()
{
    QString path = getDirDialog( tr("Select directory"), value->text() );
    value->setText( path );
    on_value_editingFinished();
}

void PathVal::updtValues()
{
    QString text = m_property->getValStr();
    value->setText( text );
}
