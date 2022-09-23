/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PIC14ECORE_H
#define PIC14ECORE_H

#include "picmrcore.h"

class MAINMODULE_EXPORT Pic14eCore : public PicMrCore
{
    public:
        Pic14eCore( eMcu* mcu );
        ~Pic14eCore();

        //virtual void reset();

    protected:
        virtual void runStep( uint16_t instr ) override;
        virtual void setBank( uint8_t bank ) override { PicMrCore::setBank( bank ); }

        uint8_t* m_FSR0L;
        uint8_t* m_FSR0H;
        uint8_t* m_FSR1L;
        uint8_t* m_FSR1H;
        uint8_t* m_BSR;

        uint16_t getFSR0() { return *m_FSR0L+(*m_FSR0H<<8); }
        void setFSR0( uint16_t fsr0 )
        {
            *m_FSR0L = fsr0 & 0x00FF;
            *m_FSR0H = (fsr0 & 0xFF00)>>8;
        }
        uint16_t getFSR1() { return *m_FSR1L+(*m_FSR1H<<8); }
        void setFSR1( uint16_t fsr1 )
        {
            *m_FSR1L = fsr1 & 0x00FF;
            *m_FSR1H = (fsr1 & 0xFF00)>>8;
        }

        virtual uint8_t GET_RAM( uint16_t addr ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            if( addr == 0 )        // INDF0
            {
                addr = getFSR0();
                if( addr & 1<<15 ) // Read Program Memory
                {
                    addr &= ~(1<<15);
                    return m_progMem[addr];
                }
            }
            else if( addr == 1 )   // INDF1
            {
                addr = getFSR1();
                if( addr & 1<<15 ) // Read Program Memory
                {
                    addr &= ~(1<<15);
                     return m_progMem[addr];
                }
            }
            return McuCpu::GET_RAM( addr );
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );
            if( addr == m_PCLaddr ) setPC( v + (m_dataMem[m_PCHaddr]<<8) ); // Writting to PCL
            else if( addr == 0 ) addr = getFSR0(); // INDF0
            else if( addr == 1 ) addr = getFSR1(); // INDF1
            McuCpu::SET_RAM( addr, v );
        }

        // Miscellaneous instructions
        //inline void RESET();
        inline void CALLW();
        inline void BRW();
        inline void MOVIW_iF( uint8_t n );
        inline void MOVIW_dF( uint8_t n );
        inline void MOVIW_Fi( uint8_t n );
        inline void MOVIW_Fd( uint8_t n );
        inline void MOVWI_iF( uint8_t n );
        inline void MOVWI_dF( uint8_t n );
        inline void MOVWI_Fi( uint8_t n );
        inline void MOVWI_Fd( uint8_t n );
        inline void MOVLB( uint8_t k );

        // ALU operations: dest ← OP(f,W)
        inline void LSLF( uint8_t f, uint8_t d );
        inline void LSRF( uint8_t f, uint8_t d );
        inline void ASRF( uint8_t f, uint8_t d );
        inline void SUBWFB( uint8_t f, uint8_t d );
        inline void ADDWFC( uint8_t f, uint8_t d );

        // Operations with literal k
        inline void ADDFSR( uint8_t n, uint8_t k );
        inline void MOVLP( uint8_t k );
        inline void BRA( uint8_t k );
        inline void MOVIW( uint8_t n, uint8_t k );
        inline void MOVWI( uint8_t n, uint8_t k );
};
#endif
