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
    m_phi0Pin = mcu->getCtrlPin("P0");
    m_phi1Pin = mcu->getCtrlPin("P1");   m_phi1Pin->setPinMode( output );
    m_phi2Pin = mcu->getCtrlPin("P2");   m_phi2Pin->setPinMode( output );
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

    m_cycle = 0;
    m_incPC = false;
    m_cpuState = cpu_RESET;
    //m_cpuState = cpu_FETCH;

    m_phi1Pin->controlPin( true, true );
    m_phi2Pin->controlPin( true, true );
    m_syncPin->controlPin( true, true );

    m_phi1Pin->setDirection( true );
    m_phi2Pin->setDirection( true );
    m_syncPin->setDirection( true );

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
    //m_opAddr = addr;

    m_cpuState = cpu_EXEC; // Default

    if( m_cycle == 1 ) { readMem( addr ); } // Read first operand
    else{
        switch( m_addrMode ){
            case addr_NONE: break; // Instruction with no operand
            case addr_ABSO:{       // Absolute: address From Mem 2 bytes, value = read( address ) 1 byte
                switch( m_cycle ){
                    case 2: readMem( addr ); m_op0 = m_mcu->extMem->getData(); break; //
                    case 3: m_opAddr = (m_mcu->extMem->getData() << 8) | m_op0 ; break;
                }
            }break;
            case addr_ACCU: m_op0 = ACC;                      break; // Value = Accumulator
            case addr_IMME: m_op0 = m_mcu->extMem->getData(); break; // Immediate: From Mem 1 byte
            case addr_INDI: break;
            case addr_RELA: break;
    }   }
}

void Mcs65Core::readMem( uint16_t addr )
{
    m_cpuState = cpu_READ;
    m_mcu->extMem->read( addr );
    m_incPC = m_addrMode > addr_ACCU;
}

void Mcs65Core::Write( uint16_t addr, uint8_t val )
{
    switch( m_addrMode )
    {
        case addr_NONE:                            break; /// ERROR
        case addr_ACCU: ACC = val;                 break;
        default: m_mcu->extMem->write( addr, val ); m_cpuState = cpu_WRITE;
}   }

void Mcs65Core::pushStack8( uint8_t byte ) { Write( 0x0100 + m_SP, byte ); m_SP--; }
void Mcs65Core::popStack8()                { m_SP++; readMem( 0x0100 + m_SP ); m_cpuState = cpu_EXEC; }

void Mcs65Core::ADC()
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

void Mcs65Core::AND() { ACC &= m_op0; setNZ( ACC ); }

void Mcs65Core::ASL()
{
    SET_CARRY( m_op0 & 0x80 );
    m_op0 <<= 1;
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::BIT()
{
    uint8_t res = m_op0 & ACC;
    SET_NEGATIVE( res );
    *m_STATUS = (*m_STATUS & 0x3F) | (m_op0 & 0xC0) | CONSTANT | BREAK;
    SET_ZERO( res == 0 );
}

void Mcs65Core::BRK() // Break/Interrupt : 1 byte, 7 cycles
{SET_INTERRUPT( 1 );
    switch( m_cycle ){
        case 2: pushStack8( ++PC >> 8 ); m_cpuState = cpu_EXEC; break; // PCH    -> Stack
        case 3: pushStack8( PC & 0xFF ); m_cpuState = cpu_EXEC; break; // PCL    -> Stack
        case 4: pushStack8( *m_STATUS ); m_cpuState = cpu_EXEC; break; // Status -> Stack
        case 5: Read( m_IsrH );          m_cpuState = cpu_EXEC; break; // Read IsrH
        case 6: Read( m_IsrL ); m_op1 = m_op0; m_cpuState = cpu_EXEC; break; // Read IsrL
        case 7: PC = (m_op1 << 8) + m_op0;            // Jump to Isr
}   }

void Mcs65Core::CLC() { SET_CARRY(0);     }
void Mcs65Core::CLD() { SET_DECIMAL(0);   }
void Mcs65Core::CLI() { SET_INTERRUPT(0); }
void Mcs65Core::CLV() { SET_OVERFLOW(0);  }

void Mcs65Core::CMP() { SET_CARRY(  ACC   >= m_op0 ); setNZ(  ACC    - m_op0 ); }
void Mcs65Core::CPX() { SET_CARRY( m_regX >= m_op0 ); setNZ( m_regX - m_op0 ); }
void Mcs65Core::CPY() { SET_CARRY( m_regY >= m_op0 ); setNZ( m_regY - m_op0 ); }

void Mcs65Core::DEC() { m_op0  -= 1; setNZ( m_op0 );  Write( m_opAddr, m_op0 ); }
void Mcs65Core::DEX() { m_regX -= 1; setNZ( m_regX ); }
void Mcs65Core::DEY() { m_regY -= 1; setNZ( m_regY ); }

void Mcs65Core::EOR() { ACC = ACC ^ m_op0; setNZ( m_op0 ); }

void Mcs65Core::INC() { m_op0  += 1; setNZ( m_op0 ); Write( m_opAddr, m_op0 ); }
void Mcs65Core::INX() { m_regX += 1; setNZ( m_regX ); }
void Mcs65Core::INY() { m_regY += 1; setNZ( m_regY ); }

void Mcs65Core::JMP() { PC = m_op0; }

void Mcs65Core::JSR()
{
    switch( m_cycle ){
        case 3: pushStack8( (PC+1) >> 8 );   m_cpuState = cpu_EXEC; break;  // PCH -> Stack
        case 4: pushStack8( (PC+1) & 0xFF ); m_cpuState = cpu_EXEC; break;  // PCL -> Stack
        case 5: PC = m_opAddr;                                              // Jump to Subroutine
}   }

void Mcs65Core::LDA() { ACC    = m_op0; setNZ( m_op0 ); }
void Mcs65Core::LDX() { m_regX = m_op0; setNZ( m_op0 ); }
void Mcs65Core::LDY() { m_regY = m_op0; setNZ( m_op0 ); }

void Mcs65Core::LSR()
{
    SET_CARRY( m_op0 & 0x01 );
    m_op0 >>= 1;
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::NOP() { return; }

void Mcs65Core::ORA() { ACC |= m_op0; setNZ( ACC ); }

void Mcs65Core::PHA() { pushStack8( ACC ); }
void Mcs65Core::PHP() { pushStack8( STATUS(C) | STATUS(B) ); }

void Mcs65Core::PLA()
{
    if( m_cycle == 2 ) popStack8();
    else             { ACC = m_op0; setNZ( ACC ); PC++; }
}

void Mcs65Core::PLP()
{
    if( m_cycle == 2 ) popStack8();
    else              { *m_STATUS = m_op0 | CONSTANT | BREAK; PC++; }
}

void Mcs65Core::ROL()
{
    bool carry = m_op0 & 0x80;
    m_op0 <<= 1;
    if( STATUS(C) ) m_op0 |= 0x01;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::ROR()
{
    bool carry = m_op0 & 1;
    m_op0 >>= 1;
    if( STATUS(C)  ) m_op0 |= 0x80;
    SET_CARRY( carry );
    setNZ( m_op0 );
    Write( m_opAddr, m_op0 );
}

void Mcs65Core::RTI() // Return from Interrupt ////
{
    switch( m_cycle ){
        case 2: popStack8();                                       break;
        case 3: popStack8(); *m_STATUS = m_op0 | CONSTANT | BREAK; break;
        case 4: popStack8(); m_op1 = m_op0;                        break;
        case 5: PC = (m_op0 << 8) | m_op1;
}   }

void Mcs65Core::RTS() // Return from Subroutine
{
    switch( m_cycle ){
        case 2: popStack8();                break;
        case 3: popStack8(); m_op0 = m_mcu->extMem->getData(); break;
        case 4: PC = ( m_op0 << 8) | m_mcu->extMem->getData() ;//( (m_op0 << 8) | m_op1 ) + 1;
}   }

void Mcs65Core::SBC()
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

void Mcs65Core::SEC() { SET_CARRY(1); }
void Mcs65Core::SED() { SET_DECIMAL(1); }
void Mcs65Core::SEI() { SET_INTERRUPT(1); }

void Mcs65Core::STA() { Write( m_opAddr,  ACC ); }
void Mcs65Core::STX() { Write( m_opAddr, m_regX ); }
void Mcs65Core::STY() { Write( m_opAddr, m_regY ); }

void Mcs65Core::TAX() { m_regX = ACC; setNZ( ACC ); }
void Mcs65Core::TAY() { m_regY = ACC; setNZ( ACC ); }

void Mcs65Core::TXA() { ACC = m_regX; setNZ( ACC ); }
void Mcs65Core::TYA() { ACC = m_regY; setNZ( ACC ); }

void Mcs65Core::TSX() { m_regX = m_SP; setNZ( m_SP ); }
void Mcs65Core::TXS() { m_SP = m_regX; }

void  Mcs65Core::BXX( uint8_t flag, uint8_t y ) /// +1 cycle same page +2 different page
{
    uint8_t bit = 0;
    switch( flag ) {
        case 0: bit = N; break;
        case 1: bit = O; break;
        case 2: bit = C; break;
        case 3: bit = Z; break;
    }
    uint8_t flagVal = STATUS( bit ) ? 1 : 0;
    if( flagVal == y ) PC = m_op0;
}

void Mcs65Core::runDecoder()
{
    if( m_incPC ) { PC++; m_incPC = false; }
    m_mcu->cyclesDone = 1;
    m_cycle++;

    m_phi2Pin->sheduleState( false, 0 );
    m_phi2Pin->sheduleState( true, m_psStep ); // ClkPin High at half cycle

    if( m_cpuState == cpu_RESET )              //Reset proccess (8 cycles)
    {
        switch( m_cycle ){
            //case 1:                                return;
            //case 2: return;//m_mcu->extMem->read( 1 );      return;
            //case 3: return;//popStack8();                   return;
            //case 4: return;//popStack8();                   return;
            //case 5: return;//popStack8();                   return;
            case 6: m_mcu->extMem->read( 0XFFFC ); return;
            case 7:{
                PC = m_mcu->extMem->getData();
                m_mcu->extMem->read( 0XFFFD );
                return;
            }
            case 8:{
                uint32_t data = m_mcu->extMem->getData();
                PC += data << 8;
                m_cpuState = cpu_FETCH;   // Reset sequence finished, go fetch first instruction.
            }
            default: return;
        }
    }

    if( m_cpuState == cpu_DECODE )            // Decode Instruction
    {
        m_ctrlPC   = false;    // Increment PC at execution
        m_cpuState = cpu_READ;
        m_addrMode = addr_NONE;
        m_syncPin->sheduleState( false, 0 ); /// prop delay???

        m_instr = m_mcu->extMem->getData();
        switch( m_instr ) {                   // Irregular Instructions
            case 0x00:  m_ctrlPC = true; break; // BRK
            case 0x08:  break; // PHP
            case 0x18:  break; // CLC
            case 0x20:  m_addrMode = addr_ABSO; m_ctrlPC = true; break; // JSR abs Execution controls PC
            case 0x28:  m_ctrlPC = true; break; // PLP
            case 0x38:  break; // SEC
            case 0x40:  m_ctrlPC = true; break; // RTI
            case 0x48:  break; // PHA
            case 0x58:  break; // CLI
            case 0x60:  m_ctrlPC = true; break; // RTS
            case 0x68:  m_ctrlPC = true; break; // PLA
            case 0x78:  break; // SEI
            case 0x88:  break; // DEY
            case 0x8A:  break; // TXA
            case 0x98:  break; // TYA
            case 0x9A:  break; // TXS
            case 0xA8:  break; // TAY
            case 0xAA:  break; // TAX
            case 0xB8:  break; // CLV
            case 0xBA:  break; // TSX
            case 0xC8:  break; // INY
            case 0xCA:  break; // DEX
            case 0xD8:  break; // CLD
            case 0xE8:  break; // INX
            case 0xEA:  break; // NOP
            case 0xF8:  break; // SED
            default:                          // Regular Instructions
            {                                 // OOOAAAGG , O = Opcode, A = Addr mode, G = Group
                m_group =  m_instr & 0b00000011;
                m_Amode = (m_instr & 0b00011100) >> 2;
                m_Ocode = (m_instr & 0b11100000) >> 5;

                if( m_group == 3 ) { m_cpuState = cpu_FETCH; return; }/// Not valid (by now)

                switch( m_Amode ){
                    case 0: {
        if( m_group == 1 ){ m_addrMode = addr_INDI; m_regI = &m_regX; m_zeroPage = true; break; }   // Group 1
                      else{ m_addrMode = addr_IMME;                                     break; }   // Groups 2, 4
                            }
                    case 1: m_addrMode = addr_ABSO;                  m_zeroPage = true; break;     // All
                    case 2: {
        if( m_group == 1 ){ m_addrMode = addr_IMME;                                     break; }   // Group 1
        if( m_group == 2 ){ m_addrMode = addr_ACCU;                                     break; }   // Group 2
                            }
                    case 3: m_addrMode = addr_ABSO;                                     break;     // All
                    case 4:
        if( m_group == 1 ){ m_addrMode = addr_INDI; m_regI = &m_regY;                    break; }   // Group 1

                    case 5: m_addrMode = addr_RELA; m_regI = &m_regX; m_zeroPage = true; break;     // same
                    case 6:
        if( m_group == 1 ){ m_addrMode = addr_ABSO; m_regI = &m_regY;                    break; }   // Group 1

                    case 7: m_addrMode = addr_ABSO; m_regI = &m_regX;                    break;     // All

                   default: m_addrMode = addr_NONE; m_regI = NULL  ; m_zeroPage = false;
                }
            }
        }
    }
    if( m_cpuState == cpu_READ ) Read( PC+1 );

    if( m_cpuState == cpu_EXEC )             // We are executing m_instr
    {
        m_cpuState = cpu_FETCH; // Default, Ops can change this
        if( !m_ctrlPC ) PC++;

        switch( m_instr ) {                 // Irregular m_instrs
            case 0x00: BRK(); break; // BRK
            case 0x08: PHP(); break; // PHP
            case 0x18: CLC(); break; // CLC
            case 0x20: JSR(); break; // JSR abs
            case 0x28: PLP(); break; // PLP
            case 0x38: SEC(); break; // SEC
            case 0x40: RTI(); break; // RTI
            case 0x48: PHA(); break; // PHA
            case 0x58: CLI(); break; // CLI
            case 0x60: RTS(); break; // RTS
            case 0x68: PLA(); break; // PLA
            case 0x78: SEI(); break; // SEI
            case 0x88: DEY(); break; // DEY
            case 0x8A: TXA(); break; // TXA
            case 0x98: TYA(); break; // TYA
            case 0x9A: TXS(); break; // TXS
            case 0xA8: TAY(); break; // TAY
            case 0xAA: TAX(); break; // TAX
            case 0xB8: CLV(); break; // CLV
            case 0xBA: TSX(); break; // TSX
            case 0xC8: INY(); break; // INY
            case 0xCA: DEX(); break; // DEX
            case 0xD8: CLD(); break; // CLD
            case 0xE8: INX(); break; // INX
            case 0xEA: NOP(); break; // NOP
            case 0xF8: SED(); break; // SED
            default:{
                switch( m_group ){               // Regular m_instrs
                    case 0:{
                        if( m_Amode == 4 ) // xxy 100 00 conditional branch
                        {
                            uint8_t flag = (m_instr & 0b11000000) >> 6; // 0=N, 1=O, 2=C, 3=Z
                            uint8_t y    = (m_instr & 0b00100000) >> 5;
                            BXX( flag, y ); // BPL BMI BVC BVS BCC BCS BNE BEQ
                        }else{
                            switch( m_Ocode ){
                                case 0: break;
                                case 1: BIT(); break;
                                case 2: //JMP();
                                case 3: JMP(); break;
                                case 4: STY(); break;
                                case 5: LDY(); break;
                                case 6: CPY(); break;
                                case 7: CPX(); break;
                        }   }
                    } break;
                    case 1:{ switch( m_Ocode ){
                            case 0: ORA(); break;
                            case 1: AND(); break;
                            case 2: EOR(); break;
                            case 3: ADC(); break;
                            case 4: STA(); break;
                            case 5: LDA(); break;
                            case 6: CMP(); break;
                            case 7: SBC(); break;
                    } }break;
                    case 2:{ switch( m_Ocode ){
                            case 0: ASL(); break;
                            case 1: ROL(); break;
                            case 2: LSR(); break;
                            case 3: ROR(); break;
                            case 4: STX(); break;
                            case 5: LDX(); break;
                            case 6: DEC(); break;
                            case 7: INC(); break;
                    } }break;
                    case 3: break;
    }   }   }   }

    if( m_cpuState == cpu_FETCH ) // Start m_instr cycle
    {
        m_cycle = 0;
        m_cpuState = cpu_DECODE;
        m_addrMode = addr_NONE;
        m_syncPin->sheduleState( true, 0 ); /// prop delay???

        m_mcu->extMem->read( PC ); // Fetch Instruction
    }
    else if( m_cpuState == cpu_WRITE ) m_cpuState = cpu_FETCH;
}
