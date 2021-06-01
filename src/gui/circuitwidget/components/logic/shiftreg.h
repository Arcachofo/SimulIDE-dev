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

#ifndef SHIFTREG_H
#define SHIFTREG_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT ShiftReg : public LogicComponent
{
    Q_OBJECT
    Q_PROPERTY( bool   Clock_Inverted READ clockInv  WRITE setClockInv  DESIGNABLE true USER true )
    Q_PROPERTY( bool   Reset_Inverted READ resetInv  WRITE setResetInv  DESIGNABLE true USER true )
    Q_PROPERTY( bool   Tristate       READ tristate                                     USER true )

    public:
        ShiftReg( QObject* parent, QString type, QString id );
        ~ShiftReg();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        bool resetInv() { return m_resetInv; }
        void setResetInv( bool inv );

        bool tristate() { return true; }

    private:
        bool m_resetInv;
};

#endif

