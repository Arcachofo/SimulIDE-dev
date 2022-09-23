/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "labelval.h"

LabelVal::LabelVal( QWidget* parent )
        : QWidget( parent )
{
    setupUi( this );
}

void LabelVal::setLabelVal( QString caption )
{
    label->setText( caption );
}

