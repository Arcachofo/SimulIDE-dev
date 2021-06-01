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

#ifndef BINCOUNTER_H
#define BINCOUNTER_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT BinCounter : public LogicComponent
{
    Q_OBJECT
    Q_PROPERTY( bool   Pin_SET      READ pinSet     WRITE useSetPin     DESIGNABLE true USER true )
    Q_PROPERTY( bool   Clock_Inverted READ clockInv  WRITE setClockInv  DESIGNABLE true USER true )
    Q_PROPERTY( bool   Reset_Inverted READ srInv     WRITE setSrInv     DESIGNABLE true USER true )
    Q_PROPERTY( int    Max_Value      READ TopValue  WRITE setTopValue  DESIGNABLE true  USER true )

    public:
        BinCounter( QObject* parent, QString type, QString id );
        ~BinCounter();

        virtual QList<propGroup_t> propGroups() override;

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        int  TopValue() { return m_TopValue; }
        void setTopValue( int tv ) { m_TopValue = tv; }

        bool srInv() { return m_resetInv; }
        void setSrInv( bool inv );

        bool pinSet() { return m_pinSet; }
        void useSetPin( bool set );

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

    protected:
        int m_Counter;
        int m_TopValue;

        bool m_resetInv;
        bool m_pinSet;

        IoPin* m_setPin;
        IoPin* m_resetPin;
};

#endif
