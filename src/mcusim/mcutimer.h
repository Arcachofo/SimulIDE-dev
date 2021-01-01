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

#include "e-element.h"
#include "regsignal.h"

#define TIM_COUNT_L m_countL ? m_countL[0] : 0
#define TIM_COUNT_H m_countH ? m_countH[0] : 0

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuTimer : public eElement
{
        friend class McuCreator;

    public:
        McuTimer( eMcu* mcu );
        ~McuTimer();

 static void remove();
 static McuTimer* getTimer( QString name ) { return m_timers.value( name ); }

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void sheduleEvents();
        virtual void enable( uint8_t en );
        virtual void configure( uint8_t val ){;}
        virtual void countWriteL( uint8_t val );
        virtual void countWriteH( uint8_t val );
        virtual void countReadL( uint8_t val );
        virtual void countReadH( uint8_t val );
        virtual void updtCycles(){;}
        virtual void updtCount();

        QString name() { return m_name; }

        //Signals:
        RegSignal<uint8_t> on_tov;
        RegSignal<uint8_t> on_comp;

    protected:

        QString m_name;

        eMcu*   m_mcu;
        std::vector<McuPin*> m_ocPin; // Output Compare Pins

        int      m_nBits;
        uint16_t m_prescaler;

        bool m_running;  // is Timer running?
        bool m_compare;  // are we comparing?
        bool m_bidirec;  // is Timer bidirectional?
        bool m_reverse;  // is Timer counting backwards?

        uint8_t m_clkSrc;  // Source of Timer clock

        uint8_t* m_countL; // Actual ram for counter low byte
        uint8_t* m_countH; // Actual ram for counter high byte

        uint32_t m_countVal;  // Value of counter
        uint16_t m_countValL; // Value of counter low byte
        uint16_t m_countValH; // Value of counter high byte

        uint16_t m_ovfMatch;  // counter vale to match an overflow
        uint32_t m_ovfPeriod; // overflow period
        uint64_t m_ovfCycle;  // absolute cycle of next overflow

        uint16_t m_comMatch;  // counter vale to match a comparator
        uint32_t m_comPeriod; // comparator period
        uint64_t m_comCycle;  // absolute cycle of next compartion


 static QHash<QString, McuTimer*> m_timers;// Access TIMERS by name
};


#endif
