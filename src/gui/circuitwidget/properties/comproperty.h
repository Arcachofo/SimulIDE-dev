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

#ifndef COMPROPERTY_H
#define COMPROPERTY_H

#include <QString>

#include "propdialog.h"

class MAINMODULE_EXPORT ComProperty
{
    public:
        ComProperty( QString name, QString caption, QString unit, QString type );
        virtual ~ComProperty(){;}

        QString name() { return m_name; }
        QString capt() { return m_capt; }
        QString type() { return m_type; }
        QString unit() { return m_unit; }

        void setUnit( QString u )  // Old: TODELETE
        { setValStr( getValue()+" "+u ); }

        virtual void    setValStr( QString val ){;}
        virtual QString getValStr(){return "";}
        virtual QString getValue(){return "";}

        virtual PropVal* getWidget( PropDialog* dialog ){ return NULL; }

    protected:
        PropVal* m_widget;

        QString m_name;
        QString m_capt;
        QString m_type;
        QString m_unit;
        uint8_t m_flags;
};

#endif
