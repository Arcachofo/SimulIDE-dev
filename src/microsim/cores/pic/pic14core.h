/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PIC14CORE_H
#define PIC14CORE_H

#include "picmrcore.h"

class Pic14Core : public PicMrCore
{
    public:
        Pic14Core( eMcu* mcu );
        ~Pic14Core();

protected:
    virtual void setBank( uint8_t bank ) override { PicMrCore::setBank( bank ); }

        uint8_t* m_FSR;
        uint8_t m_WregHidden;

        virtual uint8_t GET_RAM( uint16_t addr ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            if( addr == 0 ) addr = getINDF();// INDF
            return McuCpu::GET_RAM( addr );
        }
        virtual void SET_RAM( uint16_t addr, uint8_t v ) override //
        {
            addr = m_mcu->getMapperAddr( addr+m_bank );

            if( addr == m_PCLaddr ) setPC( v + (m_dataMem[m_PCHaddr]<<8) ); // Writting to PCL
            else if( addr == 0 ) addr = getINDF();      // INDF

            McuCpu::SET_RAM( addr, v );
        }
        inline uint16_t getINDF()
        {
            uint16_t  addr = *m_FSR;
            if( *m_STATUS & 1<<IRP ) addr |= 1<<8;
            return addr;
        }
};

#endif
