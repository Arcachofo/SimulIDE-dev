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

#include "cpu.h"

Cpu::Cpu( eMcu* mcu )
   : CpuBase( mcu )
   , CpuExtMem( mcu )
{
}
Cpu::~Cpu() {}

void Cpu::reset()
{
    m_state = sFETCH;
    m_nextClock = true;
}

void Cpu::extClock( bool clkState ) // External Clock
{
    if( clkState == m_nextClock ) runStep();
}

void Cpu::runStep() // Called every 1/2 Clock cycle
{
    if( m_nextClock ) clkRising();
    else              clkFalling();
    CpuExtMem::clock( m_nextClock );
    m_nextClock = !m_nextClock;
}

void Cpu::clkRising()
{
    switch( m_state ) {
        case sRESET:            break;
        case sFETCH:  fetch();  break;
        case sDECODE: decode(); break;
        case sOPERS:  operands(); break;
        case sEXEC:   (this->*m_instExec)(); break;
        case sRESULT: result(); break;
    }
}

void Cpu::clkFalling()
{
}

void Cpu::fetch()
{
    m_opIndex = 0;
    m_nOps= 0;
    //addMemOp( m_ReadMEM, &PC ); // Read byte from Memory
    readMem( &m_PC );
    m_state = sDECODE;
}

void Cpu::decode()
{
    m_opCode = getData();

    switch( m_opCode ) {
        case 0x00:             // LDA absolute
            addrMode( aABSO );
            instType( iREOP );
            m_instExec = &Cpu::LDA;
            break;
        default:   break;
    }
    m_state = sOPERS;
}

void Cpu::operands() // Execute Op = Call function
{
    //(this->*m_opList[m_opIndex])(); // Call function

    //m_opIndex++;
    //if( m_opIndex == m_nOps ) m_state = sFETCH; // All Ops executed
}

void Cpu::execute()
{

}

void Cpu::result()
{

}

void Cpu::addrMode( addrMode_t mode )
{
    switch( mode ) {
        case aNONE: break;
        case aACCU: break;
        case aABSO:
            addMemOp( m_ReadMEM, &m_PC );
            addMemOp( m_ReadMEM, &m_PC );
            break;
        case aINDI: break;
        case aIMME: break;
        case aRELA: break;
    }
}

void Cpu::instType( instType_t type )
{
    switch ( type ) {
        case iNONE: break;
        case iJUMP: break;
        case iREOP:
            addMemOp( m_ReadMEM, &m_opAddr );
            break;
        case iRMWI: break;
        default: break;
    }
}

/*void Cpu::readMem()
{

}

void Cpu::writeMem()
{

}*/

/*void Cpu::addOp( void (Cpu::* func)() )
{
    m_opList[m_nOps] = func;
    m_nOps++;
}*/

// Instructions ------------------
void Cpu::LDA()
{

}
