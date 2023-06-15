/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "numprop.h"
#include "numval.h"
#include "utils.h"
#include "compbase.h"

const QString NumProp::getValU( double val, QString &u )
{ return QString::number( val )+" "+u; }

QString NumProp::getStr( double val )
{
    QString valStr;
    //if( m_widget ) valStr = m_widget->getValWithUnit();
    //else
    {
        double multiplier = getMultiplier( m_unit );
        valStr = QString::number( val/multiplier );
        if( !m_unit.isEmpty() ) valStr.append(" "+m_unit );
    }
    return valStr;
}

double NumProp::getVal( const QString &val, CompBase* comp )
{
    QStringList l = val.split(" ");
    double  v = l.first().toDouble();

    if( l.size() > 1 )
    {
        QString unit = l.last();
        if( !unit.startsWith("_") ) m_unit = unit;
    }
    if( comp->showProp() == m_name ) comp->setValLabelText( QString::number( v )+" "+m_unit );

    return  v*getMultiplier( m_unit );
}
