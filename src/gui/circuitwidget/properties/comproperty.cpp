/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "comproperty.h"
#include "propval.h"

ComProperty::ComProperty( QString name, QString caption, QString unit, QString type )
{
    m_name = name;
    m_capt = caption;
    m_unit = unit;
    m_type = type;

    m_widget = NULL;
}

void ComProperty::setWidget( PropVal* w )
{
    m_widget = w;
}

