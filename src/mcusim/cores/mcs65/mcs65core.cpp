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

#include "mcs65core.h"
#include "extmem.h"

Mcs65Core::Mcs65Core( eMcu* mcu )
         : McuCore( mcu )
{
    // Control Pins
    m_phi0Pin = mcu->getCtrlPin("P0"); m_phi0Pin->setPinMode( output );
    m_phi1Pin = mcu->getCtrlPin("P1"); m_phi1Pin->setPinMode( output );
    m_phi2Pin = mcu->getCtrlPin("P2"); m_phi2Pin->setPinMode( output );
    m_syncPin = mcu->getCtrlPin("SYNC"); m_syncPin->setPinMode( output );
    // Interrupt Pins
    m_irqPin = mcu->getCtrlPin("IRQ");
    m_nmiPin = mcu->getCtrlPin("NMI");
    // User Pins
    m_rdyPin = mcu->getCtrlPin("RDY");
    m_soPin  = mcu->getCtrlPin("SO");
    /// m_dbePin = McuPort::getPin("DBE"););
}
Mcs65Core::~Mcs65Core() {}

void Mcs65Core::reset()
{
    McuCore::reset();

    m_phi2Pin->sheduleState( true, 1 );
    m_cpuState = cpu_FETCH;

    m_psStep = m_mcu->psCycle()/2;
    m_mcu->extMem->setAddrSetTime(   50*m_psStep/100 );  // Addr pins
    m_mcu->extMem->setReadSetTime(  150*m_psStep/100 );  //
    m_mcu->extMem->setWriteSetTime( 120*m_psStep/100 );  //
    m_mcu->extMem->setReadBusTime(  160*m_psStep/100 );  //
}

void Mcs65Core::setNZ( uint8_t val )
{
    SET_NEGATIVE( val & 0x80 );
    SET_ZERO( val == 0 );
}

void Mcs65Core::Read( uint16_t addr )
{
    m_readAddr = addr;

    m_cpuState = cpu_EXEC; // Default

    if( m_step == 0 ) { m_cpuState = cpu_READ; m_mcu->extMem->read( m_readAddr ); }
    else{
        switch( m_addrMode )
        {
            case addr_NONE: break; /// ERROR
            case addr_ABSO:{       // Absolute: address From Mem 2 bytes, value = read( address ) 1 byte
                switch( m_step ){
                    case 1: m_cpuState = cpu_READ; m_mcu->extMem->read( m_readAddr ); m_op1 = m_op0; break; //
                    case 2: m_cpuState = cpu_READ; m_mcu->extMem->read( (m_op0 << 8) | m_op1 ); break;
                    case 3: m_cpuState = cpu_EXEC; m_op0 = m_mcu->extMem->getData();
                }
            }break;
            case addr_ACCU: m_op0 = ACC;                      break; // Value = Accumulator
            case addr_IMME: m_op0 = m_mcu->extMem->getData(); break; // Immediate: From Mem 1 byte
            case addr_INDI: break;
            case addr_RELA: break;
        }
    }
    PC++;
}

void Mcs65Core::Write( uint16_t addr, uint8_t val )
{
    switch( m_addrMode )
    {
        case addr_NONE:                            break; /// ERROR
        case addr_ACCU: ACC = val;                 break;
        default: m_mcu->extMem->write( addr, val );
    }
}

void Mcs65Core::pushStack8( uint8_t byte ) { Write( 0x0100 + m_SP, byte ); m_SP--; }
void Mcs65Core::popStack8()                { m_SP++; Read( 0x0100 + m_SP ); }

void Mcs65Core::Op_ADC()
{
    uint8_t m = m_op0;
    unsigned int tmp = m + ACC + (STATUS(C) ? 1 : 0);
    SET_ZERO( (tmp & 0xFF) == 0 );

    if( STATUS(D) )
    {
        if( ((ACC & 0xF) + (m & 0xF) + (STATUS(C) ? 1 : 0)) > 9) tmp += 6;

        SET_NEGATIVE( tmp & 0x80 );
        SET_OVERFLOW( !((ACC ^ m) & 0x80) && ((ACC ^ tmp) & 0x80) );

        if( tmp > 0x99) tmp += 96;
        SET_CARRY(tmp > 0x99);
    }else{
        SET_NEGATIVE( tmp & 0x80 );
        SET_OVERFLOW(!((ACC ^ m) & 0x80) && ((ACC ^ tmp) & 0x80));
        SET_CARRY(tmp > 0xFF);
    }
    ACC = tmp & 0xFF;
}

void Mcs65Core::Op_AND() { ACC &= m_op0; setNZ( ACC ); }

void Mcs65Core::Op_ASL()
{
    SET_CARRY( m_op0 & 0x80 );
    m_op0 <<= 1;
    setNZ( m_op0 );
    Write( m_readAddr, m_op0 );
}
/// +1 cycle same page +2 different page
void Mcs65Core::Op_BCC() { if(!STATUS(C) ) PC = m_op0; } // Branch on Carry Clear : 1b 2c+1|2
void Mcs65Core::Op_BCS() { if( STATUS(C) ) PC = m_op0; } // Branch on Carry Set   : 1b 2c+1|2
void Mcs65Core::Op_BEQ() { if( STATUS(Z) ) PC = m_op0; } // Branch on Result Zero : 1b 2c+1|2

void Mcs65Core::Op_BIT()
{
    uint8_t res = m_op0 & ACC;
    SET_NEGATIVE( res );
    *m_STATUS = (*m_STATUS & 0x3F) | (m_op0 & 0xC0) | CONSTANT | BREAK;
    SET_ZERO( res == 0 );
}

void Mcs65Core::Op_BMI() { if( STATUS(N) ) PC = m_op0; }
void Mcs65Core::Op_BNE() { if(!STATUS(Z) ) PC = m_op0; }
void Mcs65Core::Op_BPL() { if(!STATUS(N) ) PC = m_op0; }

void Mcs65Core::Op_BRK() // Break/Interrupt : 1 byte, 7 cycles
{SET_INTERRUPT( 1 );
    switch( m_step )
    {
        case 0: pushStack8( ++PC >> 8 );       break; // PCH    -> Stack
        case 1: pushStack8( PC & 0xFF );       break; // PCL    -> Stack
        case 2: pushStack8( *m_STATUS );       break; // Status -> Stack
        case 3: Read( m_IsrH );                break; // Read IsrH
        case 4: Read( m_IsrL ); m_op1 = m_op0; break; // Read IsrL
        case 5: PC = (m_op1 << 8) + m_op0;            // Jump to Isr
    }
}

void Mcs65Core::Op_BVC() { if(!STATUS(O) ) PC = m_op0; }
void Mcs65Core::Op_BVS() { if( STATUS(O) ) PC = m_op0; }

void Mcs65Core::Op_CLC() { SET_CARRY(0);     }
void Mcs65Core::Op_CLD() { SET_DECIMAL(0);   }
void Mcs65Core::Op_CLI() { SET_INTERRUPT(0); }
void Mcs65Core::Op_CLV() { SET_OVERFLOW(0);  }

void Mcs65Core::Op_CMP() { SET_CARRY(  ACC    >= m_op0 ); setNZ(  ACC    - m_op0 ); }
void Mcs65Core::Op_CPX() { SET_CARRY( *m_regX >= m_op0 ); setNZ( *m_regX - m_op0 ); }
void Mcs65Core::Op_CPY() { SET_CARRY( *m_regY >= m_op0 ); setNZ( *m_regY - m_op0 ); }

void Mcs65Core::Op_DEC() {  m_op0  -= 1; setNZ( m_op0 );  Write( m_readAddr, m_op0 ); }
void Mcs65Core::Op_DEX() { *m_regX -= 1; setNZ( *m_regX ); }
void Mcs65Core::Op_DEY() { *m_regY -= 1; setNZ( *m_regY ); }

void Mcs65Core::Op_EOR() { ACC = ACC ^ m_op0; setNZ( m_op0 ); }

void Mcs65Core::Op_INC() {  m_op0  += 1; setNZ( m_op0 ); Write( m_readAddr, m_op0 ); }
void Mcs65Core::Op_INX() { *m_regX += 1; setNZ( *m_regX ); }
void Mcs65Core::Op_INY() { *m_regY += 1; setNZ( *m_regY ); }

void Mcs65Core::Op_JMP() { PC = m_op0; }

void Mcs65Core::Op_JSR()                    /////////////////
{
    PC--;
    pushStack8( (PC >> 8) & 0xFF);
    pushStack8( PC & 0xFF );
    PC = m_op0;
}

void Mcs65Core::Op_LDA() { ACC     = m_op0; setNZ( m_op0 ); }
void Mcs65Core::Op_LDX() { *m_regX = m_op0; setNZ( m_op0 ); }
void Mcs65Core::Op_LDY() { *m_regY = m_op0; setNZ( m_op0 ); }

void Mcs65Core::Op_LSR()
{
    SET_CARRY( m_op0 & 0x01 );
    m_op0 >>= 1;
    setNZ( m_op0 );
    Write( m_readAddr, m_op0 );
}

void Mcs65Core::Op_NOP() { return; }

void Mcs65Core::Op_ORA() { ACC |= m_op0; setNZ( ACC ); }

void Mcs65Core::Op_PHA() { pushStack8( ACC ); }
void Mcs65Core::Op_PHP() { pushStack8( STATUS(C) | STATUS(B) ); }

void Mcs65Core::Op_PLA()
{
    if( m_step == 0 ) { popStack8(); m_cpuState = cpu_EXEC; return; }
    ACC = m_op0;
    setNZ( ACC );
}

void Mcs65Core::Op_PLP()
{
    if( m_step == 0 ) { popStack8(); m_cpuState = cpu_EXEC; }
    else              *m_STATUS = m_op0 | CONSTANT | BREAK;
}

void Mcs65Core::Op_ROL()
{
    bool carry = m_op0 & 0x80;
    m_op0 <<= 1;
    if( STATUS(C) ) m_op0 |= 0x01;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_readAddr, m_op0 );
}

void Mcs65Core::Op_ROR()
{
    bool carry = m_op0 & 1;
    m_op0 >>= 1;
    if( STATUS(C)  ) m_op0 |= 0x80;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_readAddr, m_op0 );
}

void Mcs65Core::Op_RTI() // Return from Interrupt
{
    switch( m_step )
    {
        case 0: popStack8(); m_cpuState = cpu_EXEC; break;
        case 1: popStack8(); m_cpuState = cpu_EXEC; *m_STATUS = m_op0 | CONSTANT | BREAK; break;
        case 2: popStack8(); m_cpuState = cpu_EXEC; m_op1 = m_op0; break;
        case 3: PC = (m_op0 << 8) | m_op1;
    }
}

void Mcs65Core::Op_RTS() // Return from Subroutine
{
    switch( m_step )
    {
        case 0: popStack8(); m_cpuState = cpu_EXEC; break;
        case 1: popStack8(); m_cpuState = cpu_EXEC; m_op1 = m_op0; break;
        case 2: PC = ( (m_op0 << 8) | m_op1 ) + 1;
    }
}

void Mcs65Core::Op_SBC()
{
    uint8_t m = m_op0;
    uint tmp = ACC - m - (STATUS(C) ? 0 : 1);
    SET_NEGATIVE( tmp & 0x80 );
    SET_ZERO( (tmp & 0xFF) == 0 );
    SET_OVERFLOW( ((ACC ^ tmp) & 0x80) && ((ACC ^ m) & 0x80) );

    if( STATUS(D) )
    {
        if( ((ACC & 0x0F) - (STATUS(C)  ? 0 : 1)) < (m & 0x0F)) tmp -= 6;
        if( tmp > 0x99) tmp -= 0x60;
    }
    SET_CARRY( tmp < 0x100 );
    ACC = ( tmp & 0xFF );
}

void Mcs65Core::Op_SEC() { SET_CARRY(1); }
void Mcs65Core::Op_SED() { SET_DECIMAL(1); }
void Mcs65Core::Op_SEI() { SET_INTERRUPT(1); }

void Mcs65Core::Op_STA() { Write( m_op0,  ACC ); }
void Mcs65Core::Op_STX() { Write( m_op0, *m_regX ); }
void Mcs65Core::Op_STY() { Write( m_op0, *m_regY ); }

void Mcs65Core::Op_TAX() { *m_regX = ACC; setNZ( ACC ); }
void Mcs65Core::Op_TAY() { *m_regY = ACC; setNZ( ACC ); }

void Mcs65Core::Op_TXA() { ACC = *m_regX; setNZ( ACC ); }
void Mcs65Core::Op_TYA() { ACC = *m_regY; setNZ( ACC ); }

void Mcs65Core::Op_TSX() { *m_regX = m_SP; setNZ( m_SP ); }
void Mcs65Core::Op_TXS() { m_SP = *m_regX; }

void Mcs65Core::runDecoder()
{
    m_mcu->cyclesDone = 1;

    m_phi2Pin->sheduleState( false, 0 );
    m_phi2Pin->sheduleState( true, m_psStep ); // ClkPin High at half cycle

    if( m_cpuState == cpu_DECODE )                     // Decode Instruction
    {                                                  // OOOAAAGG , O = opcode, A = Addr mode, G = group
        m_cpuState = cpu_READ;
        m_syncPin->sheduleState( false, 0 ); /// prop delay???

        uint8_t    instruction = m_mcu->extMem->getData();
        m_group =  instruction & 0b00000011;
        m_Amode = (instruction & 0b00011100) >> 2;
        m_Ocode = (instruction & 0b11100000) >> 5;
        m_step = 0;

        if( m_group == 3 ) { m_cpuState = cpu_FETCH; return; }/// Not valid (by now)

        switch( m_Amode )
        {
            case 0: {
if( m_group == 1 ){ m_addrMode = addr_INDI; m_regI = m_regX; m_zeroPage = true; break; }   // Group 1
              else{ m_addrMode = addr_IMME;                                     break; }   // Groups 2, 4
                    }
            case 1: m_addrMode = addr_ABSO;                  m_zeroPage = true; break;     // All
            case 2: {
if( m_group == 1 ){ m_addrMode = addr_IMME;                                     break; }   // Group 1
if( m_group == 2 ){ m_addrMode = addr_ACCU;                                     break; }   // Group 2
                    }
            case 3: m_addrMode = addr_ABSO;                                     break;     // All
            case 4:
if( m_group == 1 ){ m_addrMode = addr_INDI; m_regI = m_regY;                    break; }   // Group 1

            case 5: m_addrMode = addr_RELA; m_regI = m_regX; m_zeroPage = true; break;     // same
            case 6:
if( m_group == 1 ){ m_addrMode = addr_ABSO; m_regI = m_regY;                    break; }   // Group 1

            case 7: m_addrMode = addr_ABSO; m_regI = m_regX;                    break;     // All

           default: m_addrMode = addr_NONE; m_regI = NULL  ; m_zeroPage = false;
        }
    }

    if( m_cpuState == cpu_READ ) Read( PC );

    if( m_cpuState == cpu_EXEC )             // We are executing instruction
    {
        //m_op0 = m_mcu->extMem->getData(); // Read first operand (even if not used)
        m_cpuState = cpu_FETCH; // Default, Ops can change this

        switch( m_group )                // Regular instructions
        {
            case 0:{ switch( m_Ocode ){
                    case 0: break;
                    case 1: Op_BIT();  break;
                    case 2: //Op_JMP();
                    case 3: Op_JMP(); break;
                    case 4: Op_STY(); break;
                    case 5: Op_LDY(); break;
                    case 6: Op_CPY(); break;
                    case 7: Op_CPX(); break;
            } }break;
            case 1:{ switch( m_Ocode ){
                    case 0: Op_ORA(); break;
                    case 1: Op_AND(); break;
                    case 2: Op_EOR(); break;
                    case 3: Op_ADC(); break;
                    case 4: Op_STA(); break;
                    case 5: Op_LDA(); break;
                    case 6: Op_CMP(); break;
                    case 7: Op_SBC(); break;
            } }break;
            case 2:{ switch( m_Ocode ){
                    case 0: Op_ASL(); break;
                    case 1: Op_ROL(); break;
                    case 2: Op_LSR(); break;
                    case 3: Op_ROR(); break;
                    case 4: Op_STX(); break;
                    case 5: Op_LDX(); break;
                    case 6: Op_DEC(); break;
                    case 7: Op_INC(); break;
            } }break;
            case 3: break;
        }
    }

    if( m_cpuState == cpu_FETCH ) // Start instruction cycle
    {
        m_cpuState = cpu_DECODE;
        m_syncPin->sheduleState( true, 0 ); /// prop delay???

        m_mcu->extMem->read( PC ); // Fetch instruction
        PC++;
    }

    m_step++;

    /*switch( opCode )
    {
        case 0x00: //BRK
        case 0x01: //ORA

        case 0x05: //ORA
        case 0x06: //ASL

        case 0x08: //PHP
        case 0x09: //ORA
        case 0x0A: //ASL

        case 0x0D: //ORA
        case 0x0E: //ASL

        case 0x10: //BPL
        case 0x11: //ORA

        case 0x15: //ORA
        case 0x16: //ASL

        case 0x18: //CLC
        case 0x19: //ORA

        case 0x1D: //ORA
        case 0x1E: //ASL

        case 0x20: //JSR
        case 0x21: //AND

        case 0x24: //BIT
        case 0x25: //AND
        case 0x26: //ROL

        case 0x28: //PLP
        case 0x29: //AND
        case 0x2A: //ROL

        case 0x2C: //Bit
        case 0x2D: //AND
        case 0x2E: //ROL

        case 0x30: //BMI
        case 0x31: //AND

        case 0x35: //AND
        case 0x36: //ROL

        case 0x38: //SEC
        case 0x39: //AND

        case 0x3D: //AND
        case 0x3E: //ROL

        case 0x40: //RTI
        case 0x41: //EOR

        case 0x45: //EOR
        case 0x46: //LSR

        case 0x48: //PHA
        case 0x49: //EOR
        case 0x4A: //LSR

        case 0x4C: //JMP
        case 0x4D: //EOR
        case 0x4E: //LSR

        case 0x50: //BVC
        case 0x51: //EOR

        case 0x55: //EOR
        case 0x56: //LSR

        case 0x58: //CLI
        case 0x59: //EOR

        case 0x5D: //EOR
        case 0x5E: //LSR

        case 0x60: //RTS
        case 0x61: //ADC

        case 0x65: //ADC
        case 0x66: //ROR

        case 0x68: //PLA

        case 0x69: //ADC
        case 0x6A: //ROR

        case 0x6C:
        case 0x6D: //ADC
        case 0x6E: //ROR

        case 0x70: //BVS
        case 0x71: //ADC

        case 0x75: //ADC
        case 0x76: //ROR

        case 0x78: //SEI
        case 0x79: //ADC

        case 0x7D: //ADC
        case 0x7E: //ROR

        case 0x81: //STA

        case 0x84: //STY
        case 0x85: //STA
        case 0x86: //STX

        case 0x88: //DEY

        case 0x8A: //TXA

        case 0x8C: //STY
        case 0x8D: //STA
        case 0x8E: //SDX

        case 0x90: //BCC
        case 0x91: //STA

        case 0x94: //STY
        case 0x95: //STA
        case 0x96: //STX

        case 0x98: //TYA
        case 0x99: //STA
        case 0x9A: //TXS

        case 0x9D: //STA

        case 0xA0: //LDY
        case 0xA1: //LDA
        case 0xA2: //LDX

        case 0xA4: //LDY
        case 0xA5: //LDA
        case 0xA6: //LDX

        case 0xA8: //TAY
        case 0xA9: //LDA
        case 0xAA: //TAX

        case 0xAC: //LDY
        case 0xAD: //LDA
        case 0xAE: //LDX

        case 0xB0: //BCS
        case 0xB1: //LDA

        case 0xB4: //LDY
        case 0xB5: //LDA
        case 0xB6: //LDX

        case 0xB8: //CLV
        case 0xB9: //LDA
        case 0xBA: //TSX

        case 0xBC: //LDY
        case 0xBD: //LDA
        case 0xBE: //LDX

        case 0xC0: //CPY
        case 0xC1: //CMP

        case 0xC4: //CPY
        case 0xC5: //CMP
        case 0xC6: //DEC

        case 0xC8: //INY
        case 0xC9: //CMP
        case 0xCA: //DEX

        case 0xCC: //CPY
        case 0xCD: //CMP
        case 0xCE: //DEC

        case 0xD0: //BNE
        case 0xD1: //CMP

        case 0xD5: //CMP
        case 0xD6: //DEC

        case 0xD8: //CLD
        case 0xD9: //CMP

        case 0xDE: //DEC
        case 0xDD: //CMP

        case 0xE0: //CMP
        case 0xE1: //SBC

        case 0xE4: //CPX
        case 0xE5: //SBC
        case 0xE6: //INC

        case 0xE8: //INX
        case 0xE9: //SBC
        case 0xEA: //NOP

        case 0xEC: //CPX
        case 0xED: //SBC
        case 0xEE: //INC

        case 0xF0: //BEQ
        case 0xF1: //SBC

        case 0xF5: //SBC
        case 0xF6: //INC

        case 0xF8: //SED
        case 0xF9: //SBC

        case 0xFD: //SBC
        case 0xFE: //INC
    }*/
}
