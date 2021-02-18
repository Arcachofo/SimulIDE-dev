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
#include "e-logic_device.h"

class LibraryItem;

class MAINMODULE_EXPORT BinCounter : public LogicComponent, public eLogicDevice
{
    Q_OBJECT
    Q_PROPERTY( quint64 Tpd_ps  READ propDelay   WRITE setPropDelay   DESIGNABLE true USER true )
    Q_PROPERTY( quint64  Tr_ps READ riseTime WRITE setRiseTime DESIGNABLE true USER true )
    Q_PROPERTY( quint64  Tf_ps READ fallTime WRITE setFallTime DESIGNABLE true USER true )
    Q_PROPERTY( double Input_High_V READ inputHighV WRITE setInputHighV DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Low_V  READ inputLowV  WRITE setInputLowV  DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Imped  READ inputImp   WRITE setInputImp   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_High_V   READ outHighV   WRITE setOutHighV   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Low_V    READ outLowV    WRITE setOutLowV    DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Imped    READ outImp     WRITE setOutImp     DESIGNABLE true USER true )
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

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

    protected:
        int m_Counter;
        int m_TopValue;

        bool m_resetInv;
};

#endif
