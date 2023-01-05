/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUCPU_H
#define MCUCPU_H

#include "cpubase.h"


class MAINMODULE_EXPORT McuCpu : public CpuBase
{
    public:
        McuCpu( eMcu* mcu );
        ~McuCpu();

        virtual void CALL_ADDR( uint32_t addr ) override; // Used by MCU Interrupts:: All MCUs should use or override this

    protected:
        uint8_t*  m_dataMem;
        uint32_t  m_dataMemEnd;
        uint16_t* m_progMem;
        uint32_t  m_progSize;
        uint8_t m_progAddrSize;

        uint16_t  m_lowDataMemEnd;
        uint16_t  m_regEnd;

        /*uint8_t* m_spl;     // STACK POINTER low byte
        uint8_t* m_sph;     // STACK POINTER high byte
        bool     m_spPre;   // STACK pre-increment?
        int      m_spInc;   // STACK grows up or down? (+1 or -1)*/

        virtual void setPC( uint32_t pc ) { m_PC = pc; }

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

        virtual uint8_t GET_RAM( uint16_t addr )                   // All MCUs should use this
        {
            if( addr >= m_mcu->m_regStart && addr <= m_regEnd )    // Read Register
                return m_mcu->readReg( addr );                     // and call Watchers

            else if( addr <= m_dataMemEnd ) return m_dataMem[addr];// Read Ram
            return 0;
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v )           // All MCUs should use this
        {
            if( (addr >= m_mcu->m_regStart) && (addr <= m_regEnd) )// Write Register
                m_mcu->writeReg( addr, v );                        // and call Watchers

            else if( addr <= m_dataMemEnd ) m_dataMem[addr] = v;   // Write Ram
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
        uint16_t GET_REG16_LH( uint16_t addr )
        {
            return m_dataMem[addr] + (m_dataMem[addr+1] << 8);
        }

        // STACK-------------------------------------------------

        uint16_t GET_SP()                         // All MCUs should use this (used by MCU Interrupts).
        {
            uint16_t sp = REG_SPL;
            if( m_sph ) sp |= (REG_SPH << 8);
            return  sp;
        }

        void SET_SP( uint16_t sp )                // All MCUs should use this (used by MCU Interrupts).
        {
            REG_SPL = sp & 0xFF;
            if( m_sph ) REG_SPH = (sp>>8) & 0xFF;
        }

        virtual void PUSH_STACK( uint32_t addr )  // All MCUs should use this (used by MCU Interrupts).
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
