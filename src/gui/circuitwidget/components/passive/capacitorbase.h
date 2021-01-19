/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef CAPACITORBASE_H
#define CAPACITORBASE_H

#include "e-capacitor.h"
#include "component.h"


class MAINMODULE_EXPORT CapacitorBase : public Component, public eCapacitor
{
    Q_OBJECT
    Q_PROPERTY( double Capacitance   READ capac    WRITE setCapac   DESIGNABLE true USER true )

    public:

        CapacitorBase( QObject* parent, QString type, QString id );
        ~CapacitorBase();
        
        virtual QList<propGroup_t> propGroups() override;

        double capac();
        void  setCapac( double c );
        
        void setUnit( QString un );
};

#endif

