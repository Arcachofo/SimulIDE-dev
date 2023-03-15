/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "mcs65interface.h"
#include "watcher.h"

Mcs65Interface::Mcs65Interface( eMcu* mcu )
        : CpuBase( mcu )
        , eElement( mcu->getId()+"-el" )
{
    mcu->createCpuTable();
    mcu->getCpuTable()->addVariable( "Instruction", "string" );
}
Mcs65Interface::~Mcs65Interface() {}

int Mcs65Interface::getCpuReg( QString reg ) // Called by Mcu Monitor to get Integer values
{
    return CpuBase::getCpuReg( reg );
}

QString Mcs65Interface::getStrInst( uint8_t IR ) // Called by Mcu Monitor to get String values
{
    //QString inst = "Not Implemented"; return
    switch( IR )                     // Irregular Instructions
    {
        case 0x00: return "BRK";       break;
        case 0x01: break;
        case 0x02: return "ORA (d,x)"; break;
        case 0x03: break;
        case 0x04: break;
        case 0x05: return "ORA d";     break;
        case 0x06: return "ASL d";     break;
        case 0x07: break;
        case 0x08: return "PHP";       break;
        case 0x09: return "ORA #";     break;
        case 0x0A: return "ASL A";     break;
        case 0x0B: break;
        case 0x0C: break;
        case 0x0D: return "ORA a";     break;
        case 0x0E: return "ASL a";     break;
        case 0x0F: break;

        case 0x10: return "BPL r";     break;
        case 0x11: return "ORA (d),Y"; break;
        case 0x12: break;
        case 0x13: break;
        case 0x14: break;
        case 0x15: return "ORA d,x";   break;
        case 0x16: return "ASL d,x";   break;
        case 0x17: break;
        case 0x18: return "CLC";       break;
        case 0x19: return "ORA a,Y";   break;
        case 0x1A: break;
        case 0x1B: break;
        case 0x1C: break;
        case 0x1D: return "ORA a,X";   break;
        case 0x1E: return "ASL a,X";   break;
        case 0x1F: break;

        case 0x20: return "JSR a";     break;
        case 0x21: return "AND (d,X)"; break;
        case 0x22: break;
        case 0x23: break;
        case 0x24: return "BIT d";     break;
        case 0x25: return "AND d";     break;
        case 0x26: return "ROL d";     break;
        case 0x27: break;
        case 0x28: return "PLP";       break;
        case 0x29: return "AND #";     break;
        case 0x2A: return "ROL A";     break;
        case 0x2B: break;
        case 0x2C: return "BIT a";     break;
        case 0x2D: return "AND a";     break;
        case 0x2E: return "ROL a";     break;
        case 0x2F: break;


        case 0x30: return "BMI r";     break;
        case 0x31: return "AND (d),Y"; break;
        case 0x32: break;
        case 0x33: break;
        case 0x34: break;
        case 0x35: return "AND d,X";   break;
        case 0x36: return "ROL d,X";   break;
        case 0x37: break;
        case 0x38: return "SEC";       break;
        case 0x39: return "AND a,Y";   break;
        case 0x3A: break;
        case 0x3B: break;
        case 0x3C: break;
        case 0x3D: return "AND a,X";   break;
        case 0x3E: return "ROL a,X";   break;
        case 0x3F: break;

        case 0x40: return "RTI";       break;
        case 0x41: return "EOR (d,X)"; break;
        case 0x42: break;
        case 0x43: break;
        case 0x44: break;
        case 0x45: return "EOR d";     break;
        case 0x46: return "LSR d";     break;
        case 0x47: break;
        case 0x48: return "PHA";       break;
        case 0x49: return "EOR #";     break;
        case 0x4A: return "LSR A";     break;
        case 0x4B: break;
        case 0x4C: return "JMP a";     break;
        case 0x4D: return "EOR a";     break;
        case 0x4E: return "LSR a";     break;
        case 0x4F: break;

        case 0x50: return "BVC r"     ; break;
        case 0x51: return "EOR (d),Y" ; break;
        case 0x52: break;
        case 0x53: break;
        case 0x54: break;
        case 0x55: return "EOR d,X"   ; break;
        case 0x56: return "LSR d,X"   ; break;
        case 0x57: break;
        case 0x58: break;                     // CLI
        case 0x59: return "EOR a,Y"   ; break;
        case 0x5A: break;
        case 0x5B: break;
        case 0x5C: break;
        case 0x5D: return "EOR a,X"   ; break;
        case 0x5E: return "LSR a,X"   ; break;
        case 0x5F: break;

        case 0x60: return "RTS"       ; break;
        case 0x61: return "ADC (d,X)" ; break;
        case 0x62: break;
        case 0x63: break;
        case 0x64: break;
        case 0x65: return "ADC d"     ; break;
        case 0x66: return "ROR d"     ; break;
        case 0x67: break;
        case 0x68: return "PLA";      ; break;
        case 0x69: return "ADC #"     ; break;
        case 0x6A: return "ROR A"     ; break;
        case 0x6B: break;
        case 0x6C: return "JMP (a)"   ; break;
        case 0x6D: return "ADC a"     ; break;
        case 0x6E: return "ROR a"     ; break;
        case 0x6F: break;

        case 0x70: return "BVS r"     ; break;
        case 0x71: return "ADC (d),Y" ; break;
        case 0x72: break;
        case 0x73: break;
        case 0x74: break;
        case 0x75: return "ADC d,X"   ; break;
        case 0x76 :return "ROR d,X"   ; break;
        case 0x77: break;
        case 0x78: break;           // SEI
        case 0x79: return "ADC a,Y"   ; break;
        case 0x7A: break;
        case 0x7B: break;
        case 0x7C: break;
        case 0x7D: return "ADC a,X"   ; break;
        case 0x7E: return "ROR a,X"   ; break;
        case 0x7F:             ; break;

        case 0x80: break;
        case 0x81: return "STA (d,X)" ; break;
        case 0x82: break;
        case 0x83: break;
        case 0x84: return "STY d"     ; break;
        case 0x85: return "STA d"     ; break;
        case 0x86: return "STX d"     ; break;
        case 0x87: break;
        case 0x88: return "DEY";      ; break;
        case 0x89: break;
        case 0x8A: return "TXA";      ; break;
        case 0x8B: break;
        case 0x8C: return "STY a"     ; break;
        case 0x8D: return "STA a"     ; break;
        case 0x8E: return "STX a"     ; break;
        case 0x8F:             ; break;

        case 0x90: return "BCC r"     ; break;
        case 0x91: return "STA (d),Y" ; break;
        case 0x92: break;
        case 0x93: break;
        case 0x94: return "STY d,X"   ; break;
        case 0x95: return "STA d,X"   ; break;
        case 0x96: return "STX d,Y"   ; break;
        case 0x97: break;
        case 0x98: return "TYA";      ; break;
        case 0x99: return "STA a,Y"   ; break;
        case 0x9A: return "TXS";      ; break;
        case 0x9B: break;
        case 0x9C: break;
        case 0x9D: return "STA a,X"   ; break;
        case 0x9E: break;
        case 0x9F: break;

        case 0xA0: return "LDY #"     ; break;
        case 0xA1: return "LDA (d,X)" ; break;
        case 0xA2: return "LDX #"     ; break;
        case 0xA3: break;
        case 0xA4: return "LDY d"     ; break;
        case 0xA5: return "LDA d"     ; break;
        case 0xA6: return "LDX d"     ; break;
        case 0xA7: break;
        case 0xA8: return "TAY";      ; break;
        case 0xA9: return "LDA #"     ; break;
        case 0xAA: return "TAX";      ; break;
        case 0xAB: break;
        case 0xAC: return "LDY a"     ; break;
        case 0xAD: return "LDA a"     ; break;
        case 0xAE: return "LDX a"     ; break;
        case 0xAF: break;

        case 0xB0: return "BCS r"     ; break;
        case 0xB1: return "LDA (d),Y" ; break;
        case 0xB2: break;
        case 0xB3: break;
        case 0xB4: return "LDY d,X"   ; break;
        case 0xB5: return "LDA d,X"   ; break;
        case 0xB6: return "LDX d,Y"   ; break;
        case 0xB7: break;
        case 0xB8: return "CLV";      ; break;
        case 0xB9: return "LDA a,Y"   ; break;
        case 0xBA: return "TSX";      ; break;
        case 0xBB: break;
        case 0xBC: return "LDY a,X"   ; break;
        case 0xBD: return "LDA a,X"   ; break;
        case 0xBE: return "LDX a,Y"   ; break;
        case 0xBF: break;

        case 0xC0: return "CPY #"     ; break;
        case 0xC1: return "CMP (d,X)" ; break;
        case 0xC2: break;
        case 0xC3: break;
        case 0xC4: return "CPY d"     ; break;
        case 0xC5: return "CMP d"     ; break;
        case 0xC6: return "DEC d"     ; break;
        case 0xC7: break;
        case 0xC8: return "INY";      ; break;
        case 0xC9: return "CMP #"     ; break;
        case 0xCA: return "DEX";      ; break;
        case 0xCB: break;
        case 0xCC: return "CPY a"     ; break;
        case 0xCD: return "CMP a"     ; break;
        case 0xCE: return "DEC a"     ; break;
        case 0xCF: break;

        case 0xD0: return "BNE r"     ; break;
        case 0xD1: return "CMP (d),Y" ; break;
        case 0xD2: break;
        case 0xD3: break;
        case 0xD4: break;
        case 0xD5: return "CMP d,X"   ; break;
        case 0xD6: return "DEC d,X"   ; break;
        case 0xD7: break;
        case 0xD8: return "CLD";      ; break;
        case 0xD9: return "CMP a,Y"   ; break;
        case 0xDA: break;
        case 0xDB: break;
        case 0xDC: break;
        case 0xDD: return "CMP a,X"   ; break;
        case 0xDE: return "DEC a,X"   ; break;
        case 0xDF:             ; break;

        case 0xE0: return "CPX #"     ; break;
        case 0xE1: return "SBC (d,X)" ; break;
        case 0xE2: break;
        case 0xE3: break;
        case 0xE4: return "CPX d"     ; break;
        case 0xE5: return "SBC d"     ; break;
        case 0xE6: return "INC d"     ; break;
        case 0xE7: break;
        case 0xE8: return "INX";      ; break;
        case 0xE9: return "SBC #"     ; break;
        case 0xEA: return "NOP";      ; break;
        case 0xEB: break;
        case 0xEC: return "CPX a"     ; break;
        case 0xED: return "SBC a"     ; break;
        case 0xEE: return "INC a"     ; break;
        case 0xEF: break;

        case 0xF0: return "BEQ r"     ; break;
        case 0xF1: return "SBC (d),Y" ; break;
        case 0xF2: break;
        case 0xF3: break;
        case 0xF4: break;
        case 0xF5: return "SBC d,X"   ; break;
        case 0xF6: return "INC d,X"   ; break;
        case 0xF7: break;
        case 0xF8: return "SED";      ; break;
        case 0xF9: return "SBC a,Y"   ; break;
        case 0xFA: break;
        case 0xFB: break;
        case 0xFC: break;
        case 0xFD: return "SBC a,X"   ; break;
        case 0xFE: return "INC a,X"   ; break;
        case 0xFF:             ; break;
    }
    return "Not Implemented";
}
