/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "comproperty.h"
#include "propval.h"

ComProperty::ComProperty( QString name, QString caption, QString unit, QString type, uint8_t flags )
{
    m_name  = name;
    m_capt  = caption;
    m_unit  = unit;
    m_type  = type;
    m_flags = flags;

    //m_widget = NULL;
}
QString ComProperty::name() { return m_name; }
QString ComProperty::capt() { return m_capt; }
QString ComProperty::type() { return m_type; }
QString ComProperty::unit() { return m_unit; }
uint8_t ComProperty::flags(){ return m_flags; }

void    ComProperty::setValStr( QString ){;}
QString ComProperty::getValStr(){return "";}
double  ComProperty::getValue(){return 0;}

QString ComProperty::toString(){return getValStr();}

//void ComProperty::setWidget( PropVal* w ) { m_widget = w; }

