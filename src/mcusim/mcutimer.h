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

#ifndef MCUTIMER_H
#define MCUTIMER_H

#include<QHash>

#include "mcumodule.h"
#include "e-element.h"

class eMcu;
class McuOcUnit;

class MAINMODULE_EXPORT McuTimer : public McuModule, public eElement
{
        friend class McuCreator;
        friend class McuOcUnit;

    public:
        McuTimer( eMcu* mcu, QString name );
        ~McuTimer();

        enum clkSource_t{
            clkMCU=0,
            clkEXT
        };

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void sheduleEvents();
        virtual void enable( uint8_t en );
        virtual void countWriteL( uint8_t val );
        virtual void countWriteH( uint8_t val );
        virtual void updtCycles();
        virtual void updtCount( uint8_t val=0 );

        virtual void addocUnit( McuOcUnit* ocUnit ) { m_ocUnit.emplace_back( ocUnit ); }

        QString name() { return m_name; }

    protected:
        int     m_number;

        uint16_t m_prescaler;
        uint64_t m_scale;

        std::vector<uint16_t> m_prescList; // Prescaler values

        bool m_running;  // is Timer running?
        bool m_bidirec;  // is Timer bidirectional?
        bool m_reverse;  // is Timer counting backwards?

        clkSource_t m_clkSrc;  // Source of Timer clock
        uint8_t     m_clkEdge; // Clock edge in ext pin clock

        uint8_t* m_countL; // Actual ram for counter Low byte
        uint8_t* m_countH; // Actual ram for counter High byte

        uint32_t m_countVal;   // Value of counter
        uint32_t m_countStart; // Value of counter after ovf

        uint16_t m_maxCount;  // Maximum value of the counter
        uint16_t m_ovfMatch;  // counter vale to match an overflow
        uint32_t m_ovfPeriod; // overflow period
        uint64_t m_ovfCycle;  // absolute cycle of next overflow

        uint8_t m_mode;

        std::vector<McuOcUnit*> m_ocUnit; // Output Compare Units
};

class MAINMODULE_EXPORT McuTimers
{
        friend class McuCreator;

    public:
        McuTimers( eMcu* mcu );
        ~McuTimers();

       void remove();
       McuTimer* getTimer( QString name ) { return m_timerList.value( name ); }

    protected:
       eMcu* m_mcu;

       QHash<QString, McuTimer*> m_timerList;// Access TIMERS by name
};


#endif
