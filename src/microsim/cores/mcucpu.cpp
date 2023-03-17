/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcucpu.h"

McuCpu::McuCpu( eMcu* mcu )
      : CpuBase( mcu )
{
    m_dataMem    = mcu->getRam();
    m_dataMemEnd = mcu->ramSize();
    if( m_dataMemEnd ) m_dataMemEnd--;
    m_progMem    = mcu->m_progMem.data();
    m_progSize   = mcu->flashSize();

    if( mcu->m_regStart > 0 ) m_lowDataMemEnd = mcu->m_regStart-1;
    else                      m_lowDataMemEnd = 0;

    m_regEnd = mcu->m_regEnd;
    if( m_dataMemEnd > 0 )
    {
        uint16_t sregAddr = mcu->m_sregAddr;
        if( sregAddr ) m_STATUS = &m_dataMem[sregAddr];
    }
    if     ( m_progSize <= 0xFF )     m_progAddrSize = 1;
    else if( m_progSize <= 0xFFFF )   m_progAddrSize = 2;
    else if( m_progSize <= 0xFFFFFF ) m_progAddrSize = 3;
}
McuCpu::~McuCpu() {}

void McuCpu::CALL_ADDR( uint32_t addr ) // Used by MCU Interrupts:: All MCUs should use or override this
{
    PUSH_STACK( m_PC );
    setPC( addr );
    m_mcu->cyclesDone = m_retCycles;
}
