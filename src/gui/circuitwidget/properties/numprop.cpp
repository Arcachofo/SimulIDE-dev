/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "numprop.h"
#include "numval.h"
#include "utils.h"
#include "compbase.h"

const QString NumProp::getValU( double val, QString &u )
{ return QString::number( val )+" "+u; }

QString NumProp::getStr( double val )
{
    QString valStr;
    if( m_widget ) valStr = m_widget->getValWithUnit();
    else{
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

    if( l.size() > 1 ) m_unit = l.last();
    if( comp->showProp() == m_name ) comp->setValLabelText( QString::number( v )+" "+m_unit );

    return  v*getMultiplier( m_unit );
}
