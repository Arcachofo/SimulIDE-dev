/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef GATE_H
#define GATE_H

#include "logiccomponent.h"
#include "e-element.h"

class IoPin;

class MAINMODULE_EXPORT Gate : public LogicComponent, public eElement
{
    Q_OBJECT
    //Q_PROPERTY( int    Num_Inputs   READ numInps    WRITE setNumInps    DESIGNABLE true USER true )
    Q_PROPERTY( bool   Inverted     READ inverted   WRITE setInverted   DESIGNABLE true USER true )
    Q_PROPERTY( bool Open_Collector READ openCol  WRITE setOpenCol  DESIGNABLE true USER true )
    

    public:
        Gate( QObject* parent, QString type, QString id, int inputs );
        ~Gate();

        virtual QList<propGroup_t> propGroups() override;

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        bool tristate();
        void setTristate( bool t );

        bool openCol();
        void setOpenCol( bool op );
        
        virtual void setNumInps( int inputs );

        void setInverted( bool inverted );

    protected:
        virtual bool calcOutput( int inputs );

        bool m_out;
        bool m_tristate;
        bool m_openCol;
};

#endif
