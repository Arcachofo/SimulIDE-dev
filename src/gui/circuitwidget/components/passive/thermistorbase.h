/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef THERMISTORBASE_H
#define THERMISTORBASE_H

#include "itemlibrary.h"
#include "varresbase.h"


class MAINMODULE_EXPORT ThermistorBase : public VarResBase
{
    Q_OBJECT
    //Q_PROPERTY( bool PTC     READ ptc     WRITE setPtc     DESIGNABLE true USER true )
    Q_PROPERTY( int Min_Temp READ minVal WRITE setMinVal DESIGNABLE true USER true )
    Q_PROPERTY( int Max_Temp READ maxVal WRITE setMaxVal DESIGNABLE true USER true )
    Q_PROPERTY( int Temp     READ getVal WRITE setVal    DESIGNABLE true USER true )

    public:
        ThermistorBase( QObject* parent, QString type, QString id );
        ~ThermistorBase();

        bool ptc();
        void setPtc( bool ptc );

    protected:
        bool m_ptc;
};

#endif
