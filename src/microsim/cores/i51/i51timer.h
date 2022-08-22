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

#ifndef I51TIMER_H
#define I51TIMER_H

#include "mcutimer.h"

class MAINMODULE_EXPORT I51Timer : public McuTimer
{
    public:
        I51Timer( eMcu* mcu, QString name );
        ~I51Timer();

        virtual void initialize() override;
        virtual void configureA( uint8_t newTMOD ) override;

        virtual void updtCycles() override;
        virtual void updtCount( uint8_t val=0 ) override;

    protected:
        bool m_gate;

        regBits_t m_TxM;
        regBits_t m_CTx;
        regBits_t m_GATE;
};

#endif
