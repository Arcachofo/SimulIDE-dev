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

#ifndef ESHIFTREG_H
#define ESHIFTREG_H

#include <bitset>

#include "e-logic_device.h"

class MAINMODULE_EXPORT eShiftReg : public eLogicDevice
{
    public:
        eShiftReg( QString id, int latchClk=0, int serOut=0 );
        ~eShiftReg();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        bool resetInv() { return m_resetInv; }
        void setResetInv( bool inv );

    protected:
        void createLatchClockPin();
        void createSerOutPin();
        
        std::bitset<8> m_shiftReg;
        std::bitset<8> m_latch;
        
        bool m_latchClock;
        bool m_reset;
        bool m_resetInv;

        bool m_setSerOut;
        bool m_resSerOut;
        
        eSource* m_latchClockPin;
        eSource* m_serOutPin;

};
#endif
