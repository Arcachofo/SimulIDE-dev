/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef FLIPFLOPBASE_H
#define FLIPFLOPBASE_H

#include "logiccomponent.h"
#include "e-element.h"

class MAINMODULE_EXPORT FlipFlopBase : public LogicComponent, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool   Clock_Inverted READ clockInv  WRITE setClockInv  DESIGNABLE true USER true )
    Q_PROPERTY( bool   S_R_Inverted   READ srInv     WRITE setSrInv     DESIGNABLE true USER true )
    Q_PROPERTY( trigger_t Trigger     READ trigger   WRITE setTrigger   DESIGNABLE true USER true )

    public:
        FlipFlopBase( QObject* parent, QString type, QString id );
        ~FlipFlopBase();

        virtual QList<propGroup_t> propGroups() override;

        bool srInv() { return m_srInv; }
        void setSrInv( bool inv );

        virtual void stamp() override;

    protected:
        bool m_srInv;
        bool m_Q0;

        int m_dataPins;

        IoPin* m_setPin;
        IoPin* m_resetPin;
};

#endif

