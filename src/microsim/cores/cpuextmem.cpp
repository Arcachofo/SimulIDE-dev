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

