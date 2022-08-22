/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef IntMemCore_H
#define IntMemCore_H

#include "cpubase.h"
#include "e-element.h"

class McuPin;

class MAINMODULE_EXPORT IntMemCore : public CpuBase , public eElement
{
    friend class McuCreator;

    public:
        IntMemCore( eMcu* mcu );
        ~IntMemCore();

        enum memState_t{
            mem_IDLE=0,
            mem_LAEN,
            mem_ADDR,
            mem_LADI,
            mem_DATA,
            mem_READ
        };
        enum {
            RW=1,
            RE=1<<1,
            EN=1<<2,
            LA=1<<3
        };

        //virtual void initialize() override;
        //virtual void stamp() override;
        //virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void reset() override;

        virtual void extClock( bool clkState ) override;

    protected:
        void write( bool w );
        void enableOutputs( bool en );
        void runOutputs();
        void sheduleOutPuts();

        bool m_we;
        bool m_cs;
        bool m_write;
        bool m_asynchro;

        uint8_t  m_addrH;
        uint32_t m_addr;
        uint32_t m_data;

        uint m_outValue;
        uint m_nextOutVal;
        uint64_t m_propDelay; // Propagation delay

        memState_t m_memState;

        McuPin* m_rwPin;  // !Write / Read Pin
      //McuPin* m_rePin;  // !Read Pin
        McuPin* m_cshPin; // Enable Pin
        McuPin* m_cslPin; // !Enable Pin
        McuPin* m_clkPin; // Clock Pin

        std::vector<McuPin*> m_addrPin;
        std::vector<McuPin*> m_dataPin;
};

#endif
