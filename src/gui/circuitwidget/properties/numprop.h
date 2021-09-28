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

#ifndef NUMPROP_H
#define NUMPROP_H

#include "comproperty.h"
#include "propval.h"
#include "utils.h"

class MAINMODULE_EXPORT NumProp : public ComProperty
{
    public:
        NumProp( QString name, QString caption, QString unit, QString type )
        : ComProperty( name, caption, unit, type )
        {}
        ~NumProp(){;}

    protected:
        double getVal( QString val )
        {
            QStringList l = val.split(" ");
            double  v = l.first().toDouble();
            if( l.size() > 1 ) m_unit = l.last();
            return v*getMultiplier( m_unit );
        }

        QString getStr( double val )
        {
            QString valStr;
            if( m_widget ) valStr = m_widget->getValWithUnit();
            else{
                valStr = QString::number( val );
                if( !m_unit.isEmpty() ) valStr.append(" "+m_unit );
            }
            return valStr;
        }
};

#endif
