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

class CompBase;

class MAINMODULE_EXPORT NumProp : public ComProperty
{
    public:
        NumProp( QString name, QString caption, QString unit, QString type )
        : ComProperty( name, caption, unit, type )
        {}
        ~NumProp(){;}

    protected:
        const QString getValU( double val, QString &u );

        double getVal( const QString &val, CompBase* comp );

        QString getStr( double val );
};

#endif
