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

#ifndef MCUCORE_H
#define MCUCORE_H

#include "e_mcu.h"

#define REG_SPL    m_spl[0]
#define REG_SPH    m_sph[0]

#define STATUS(bit) (*m_STATUS & (1<<bit))

class MAINMODULE_EXPORT McuCore
{
        friend class McuCreator;

    public:
        McuCore( eMcu* mcu );
        virtual ~McuCore();

        virtual void reset();
        virtual void runDecoder(){;}
        virtual void runClock( bool clkState ){;}

        uint8_t getStatus() { return *m_STATUS; }
        int getCpuReg( QString reg );

        void CALL_ADDR( uint32_t addr )
        {
            PUSH_STACK( PC );// Push current PC to stack
            setPC( addr );
            m_mcu->cyclesDone = m_retCycles;
        }
        virtual uint32_t GET_STACK()
        {
            uint32_t res = 0;
            uint16_t sp = GET_SP();
            if( !m_spPre ) sp -= m_spInc;

            for( int i=0; i<m_progAddrSize; i++, sp -= m_spInc )
                res = (res<<8) | GET_RAM( sp );

            return res;
        }

        uint32_t PC;

    protected:
        eMcu* m_mcu;

        uint8_t m_retCycles;

        uint8_t*  m_dataMem;
        uint32_t  m_dataMemEnd;
        uint16_t* m_progMem;
        uint32_t  m_progSize;
        uint8_t   m_progAddrSize;

        uint16_t  m_lowDataMemEnd;
        uint16_t  m_regEnd;

        uint8_t* m_STATUS;  // STATUS register
        uint8_t* m_spl;     // STACK POINTER low byte
        uint8_t* m_sph;     // STACK POINTER high byte
        bool     m_spPre;   // STACK pre-increment?
        int      m_spInc;   // STACK grows up or down? (+1 or -1)

        QHash<QString, uint8_t*> m_cpuRegs;

        void RETI()
        {
            m_mcu->m_interrupts.retI();
            RET();
        }
        void RET()
        {
            setPC( POP_STACK() );
            m_mcu->cyclesDone = m_retCycles;
        }

        virtual uint8_t GET_RAM( uint16_t addr ) //
        {
            if( addr > m_lowDataMemEnd && addr <= m_regEnd ) // Read Register
                return m_mcu->readReg( addr );              // and call Watchers

            else if( addr <= m_dataMemEnd ) return m_dataMem[addr]; // Read Ram
            return 0;
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v )  //
        {
            if( (addr > m_lowDataMemEnd) && (addr <= m_regEnd) ) // Write Register
                m_mcu->writeReg( addr, v );                      // and call Watchers

            else if( addr <= m_dataMemEnd ) m_dataMem[addr] = v;     // Write Ram
        }

        void clear_S_Bit( uint8_t bit) { *m_STATUS &= ~(1<<bit); }
        void set_S_Bit( uint8_t bit ) { *m_STATUS |= 1<<bit; }

        void write_S_Bit( uint8_t  bit, bool val )
        {
            if( val ) *m_STATUS |= 1<<bit; \
            else      *m_STATUS &= ~(1<<bit);
        }

        void incDefault()
        {
            setPC( PC+1 );
            m_mcu->cyclesDone += 1;
        }

        virtual void setPC( uint32_t pc ) { PC = pc; }

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
        uint16_t GET_REG16_LH( uint16_t addr )
        {
            return m_dataMem[addr] + (m_dataMem[addr+1] << 8);
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

        virtual void PUSH_STACK( uint32_t addr )
        {
            uint16_t sp = GET_SP();
            if( m_spPre ) sp += m_spInc;

            for( int i=0; i<m_progAddrSize; i++, addr>>=8, sp += m_spInc )
                SET_RAM( sp, addr & 0xFF );

            if( m_spPre )  sp -= m_spInc;
            SET_SP( sp );
        }

        virtual uint32_t POP_STACK()
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
