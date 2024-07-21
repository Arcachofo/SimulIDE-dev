/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "numprop.h"
#include "utils.h"

NumProp::NumProp( QString name, QString caption, QString unit, QString type, uint8_t flags )
       : ComProperty( name, caption, unit, type, flags )
{
}

void NumProp::setValStr( QString v )
{
    setVal( getVal( v ) );
}

QString NumProp::getValStr()
{
    return getStr( getValue() );
}

double NumProp::getVal( const QString &val )
{
    QStringList l = val.split(" ");
    double  v = l.first().toDouble();

    if( l.size() > 1 )
    {
        QString unit = l.last();
        if( !unit.startsWith("_") ) m_unit = unit;
    }
    return  v*getMultiplier( m_unit );
}

QString NumProp::getStr( double val )
{
    double multiplier = getMultiplier( m_unit );
    QString  valStr = QString::number( val/multiplier );
    if( !m_unit.isEmpty() ) valStr.append(" "+m_unit );
    return valStr;
}
