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

#ifndef CORECPU_H
#define CORECPU_H

#include "mcucore.h"
#include "mcuinterrupts.h"

#define REG_SPL    m_spl[0]
#define REG_SPH    m_sph[0]

class MAINMODULE_EXPORT CoreCpu : public McuCore
{
    public:
        CoreCpu( eMcu* mcu );
        virtual ~CoreCpu();

        virtual void reset();
        virtual void runDecoder()=0;

        void CALL_ADDR( uint32_t addr ) override
        {
            PUSH_STACK( PC );// Push current PC to stack
            PC = addr;
            m_mcu->cyclesDone = 2;
        }
        void RETI() override
        {
            Interrupt::retI();
            PC = POP_STACK();
        }
        /*void McuCore::RET()
        {
            PC = POP_STACK();
            //m_mcu->cyclesDone = 2;
        }*/

    protected:

        // Data Memory access
        /*uint8_t getDataMem( uint16_t addr ) // Whole data Mem space including Registers
        {
            if( addr > m_lowDataMemEnd && addr < m_regEnd ) // Read Register
                return m_mcu->readReg( addr );              // and call Watchers

            else if( addr <= m_dataMemEnd ) return m_dataMem[addr]; // Read Ram
            return 0;
        }
        void setDataMem( uint16_t addr, uint8_t v ) // Whole data Mem space including Registers
        {
            if( (addr > m_lowDataMemEnd) && (addr < m_regEnd) ) // Write Register
                m_mcu->writeReg( addr, v );                     // and call Watchers

            else if( addr <= m_dataMemEnd) m_dataMem[addr] = v;     // Write Ram
        }*/

        uint8_t GET_RAM( uint16_t addr ) // Ram space excluding Registers
        {
            if( addr > m_lowDataMemEnd && addr < m_regEnd ) // Read Register
                return m_mcu->readReg( addr );              // and call Watchers

            else if( addr <= m_dataMemEnd) return m_dataMem[addr]; // Read Ram
            return 0;
        }
        void SET_RAM( uint16_t addr, uint8_t v )  // Ram space excluding Registers
        {
            if( (addr > m_lowDataMemEnd) && (addr < m_regEnd) ) // Write Register
                m_mcu->writeReg( addr, v );                     // and call Watchers

            else if( addr <= m_dataMemEnd) m_dataMem[addr] = v;     // Write Ram
        }

        void SET_REG16_LH( uint16_t addr, uint16_t val )
        {
            m_mcu->writeReg( addr, val );
            m_mcu->writeReg( addr+1, val>>8 );
        }
        void SET_REG16_HL( uint16_t addr, uint16_t val )
        {
            m_mcu->writeReg( addr+1, val>>8 );
            m_mcu->writeReg( addr , val );
        }

        // STACK-------------------------------------------------

        uint16_t GET_SP()
        {
            uint16_t sp = REG_SPL;
            if( m_sph ) sp |= (REG_SPH << 8);
            return  sp;
        }

        void SET_SP( uint16_t sp )
        {
            REG_SPL = sp & 0xFF;
            if( m_sph ) REG_SPH = (sp>>8) & 0xFF;
        }

        void PUSH_STACK( uint32_t addr )
        {
            uint16_t sp = GET_SP();

            if( m_spPre ) sp += m_spInc;

            for( int i=0; i<m_progAddrSize; i++, addr>>=8, sp += m_spInc )
                SET_RAM( sp, addr & 0xFF );

            if( m_spPre )  sp -= m_spInc;

            SET_SP( sp );
        }

        uint32_t POP_STACK()
        {
            uint16_t sp = GET_SP();
            uint32_t res = 0;

            if( !m_spPre ) sp -= m_spInc;

            for( int i=0; i<m_progAddrSize; i++, sp -= m_spInc )
                res = (res<<8) | GET_RAM( sp );

            if( !m_spPre ) sp += m_spInc;

            SET_SP( sp );
            return res;
        }

        void PUSH_STACK8( uint8_t v )
        {
            uint16_t sp = GET_SP();

            if( m_spPre ) sp += m_spInc;
            SET_RAM( sp, v );
            if( !m_spPre ) sp += m_spInc;

            SET_SP( sp );
        }

        uint8_t POP_STACK8()
        {
            uint16_t sp = GET_SP();

            if( !m_spPre ) sp -= m_spInc;
            uint8_t res = GET_RAM( sp );
            if( m_spPre ) sp -= m_spInc;

            SET_SP( sp );
            return res;
        }
};

#endif
