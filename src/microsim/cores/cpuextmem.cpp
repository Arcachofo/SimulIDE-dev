/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "cpuextmem.h"
#include "e_mcu.h"
#include "mcupin.h"

CpuExtMem::CpuExtMem( eMcu* mcu )
         : eElement( mcu->getId()+"-CpuExtMem" )
{
    m_dataPort = NULL; //mcu->getPort("PORTD");
    m_addrPort = NULL; //mcu->getPort("PORTA");
}
CpuExtMem::~CpuExtMem(){}

void CpuExtMem::clock( bool rising )
{

}

void CpuExtMem::readMem( uint32_t* addr )
{
    m_memOp.addr = addr;
    m_memOp.type = m_ReadMEM;
}

void CpuExtMem::writeMem( uint32_t* addr )
{

}

void CpuExtMem::addMemOp( memOpType_t type, uint32_t* addr )
{
    memOp_t memOp;
    memOp.type = type;
    memOp.addr = addr;

    m_memOpList[m_nOps] = memOp;
    m_nOps++;
}

