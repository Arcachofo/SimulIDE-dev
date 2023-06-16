
#include "z80core.h"
#include "z80cycles.h"
#include "mcupin.h"
#include "simulator.h"
#include "ioport.h"
#include "mcu.h"
#include "watcher.h"

#include "boolprop.h"
#include "stringprop.h"

Z80Core::Z80Core( eMcu* mcu )
       : CpuBase( mcu )
       , eElement( mcu->getId()+"-Z80Core" )
{
    // Values to show in Monitor High Area (any type)
    mcu->createCpuTable();
    mcu->getCpuTable()->addRegister( "A", "uint8" );
    mcu->getCpuTable()->addRegister( "B", "uint8" );
    mcu->getCpuTable()->addRegister( "C", "uint8" );
    mcu->getCpuTable()->addRegister( "D", "uint8" );
    mcu->getCpuTable()->addRegister( "E", "uint8" );
    mcu->getCpuTable()->addRegister( "H", "uint8" );
    mcu->getCpuTable()->addRegister( "L", "uint8" );
    mcu->getCpuTable()->addRegister( "XH", "uint8" );
    mcu->getCpuTable()->addRegister( "XL", "uint8" );
    mcu->getCpuTable()->addRegister( "YH", "uint8" );
    mcu->getCpuTable()->addRegister( "YL", "uint8" );
    mcu->getCpuTable()->addRegister( "I", "uint8" );
    mcu->getCpuTable()->addRegister( "R", "uint8" );
    mcu->getCpuTable()->addRegister( "AF", "uint16" );
    mcu->getCpuTable()->addRegister( "BC", "uint16" );
    mcu->getCpuTable()->addRegister( "DE", "uint16" );
    mcu->getCpuTable()->addRegister( "HL", "uint16" );
    mcu->getCpuTable()->addRegister( "IX", "uint16" );
    mcu->getCpuTable()->addRegister( "IY", "uint16" );
    mcu->getCpuTable()->addRegister( "AF'", "uint16" );
    mcu->getCpuTable()->addRegister( "BC'", "uint16" );
    mcu->getCpuTable()->addRegister( "DE'", "uint16" );
    mcu->getCpuTable()->addRegister( "HL'", "uint16" );
    mcu->getCpuTable()->addRegister( "IR", "uint16" );
    mcu->getCpuTable()->addRegister( "SP", "uint16" );
    mcu->getCpuTable()->addRegister( "WZ", "uint16" );
    mcu->getCpuTable()->addRegister( "IFF1", "string" );
    mcu->getCpuTable()->addRegister( "IFF2", "string" );

    // Values to show in Monitor Low Area (any type)
    mcu->getCpuTable()->addVariable( "TState", "string" );
    mcu->getCpuTable()->addVariable( "TState_Int", "uint32" );
    mcu->getCpuTable()->addVariable( "MCycle", "string" );
    mcu->getCpuTable()->addVariable( "M1_Type", "string" );
    mcu->getCpuTable()->addVariable( "Op_Code", "uint8" );
    mcu->getCpuTable()->addVariable( "Instruction", "string" );
    mcu->getCpuTable()->addVariable( "Address_Bus", "uint16" );
    mcu->getCpuTable()->addVariable( "Data_Bus_In", "uint8" );
    mcu->getCpuTable()->addVariable( "Data_Bus_Out", "uint8" );
    mcu->getCpuTable()->addVariable( "Bus_Op", "string" );
    mcu->getCpuTable()->addVariable( "Bus_HighZ", "string" );

    m_STATUS = &regF;
    mcu->setStatusBits({"C","N","PV","y","H","x","Z","S"});

    // initialization Z80Core settings
    m_producer = pZilog;                 // producer is Zilog - behaviour of instructions SCF and CCF
    m_cmos = false;                      // version of processor - behaviour of instruction OUT (C),0 / OUT (C),FF
    m_ioWait = true;                     // insert one wait state during I/O operations
    m_intVector = false;                 // interrupt vector for mode 2 is read from data bus

    m_delay = 10e3; // 10 ns

    m_dataPort = mcu->getIoPort("PORTD");
    m_addrPort = mcu->getIoPort("PORTA");

    m_m1Pin     = mcu->getIoPin("M1");
    m_mreqPin   = mcu->getIoPin("MREQ");
    m_iorqPin   = mcu->getIoPin("IORQ");
    m_wrPin     = mcu->getIoPin("WR");
    m_rdPin     = mcu->getIoPin("RD");
    m_rfshPin   = mcu->getIoPin("RFSH");
    m_haltPin   = mcu->getIoPin("HALT");
    m_waitPin   = mcu->getIoPin("WAIT");
    m_intPin    = mcu->getIoPin("INT");
    m_nmiPin    = mcu->getIoPin("NMI");
    m_busreqPin = mcu->getIoPin("BUSRQ");
    m_busacPin  = mcu->getIoPin("BUSAK");
    m_resetPin  = mcu->getIoPin("RESET");

    m_enumUids = QStringList()
        << "Zilog"
        << "NEC"
        << "ST";
    m_enumNames = m_enumUids;

mcu->component()->addPropGroup( { QObject::tr("Cpu"), {
//new BoolProp  <Z80Core>( "Ext_Clock", tr("External clock source"), "", this, &Z80Core::extClkEnabled, &Z80Core::enableExtClk ),
//new DoubProp  <Z80Core>( "Frequency", tr("Frequency"), "MHz" , this, &Z80Core::freq, &Z80Core::setFreq ),
new StrProp <Z80Core>( "Producer"        , QObject::tr("Producer")             , "", this, &Z80Core::producer , &Z80Core::setProducer,0,"enum" ),
new BoolProp<Z80Core>( "CMOS"            , QObject::tr("CMOS")                 , "", this, &Z80Core::cmos     , &Z80Core::setCmos ),
new BoolProp<Z80Core>( "Single cycle I/O", QObject::tr("Single cycle I/O")     , "", this, &Z80Core::ioWait   , &Z80Core::setIoWait ),
new BoolProp<Z80Core>( "Int_Vector"      , QObject::tr("Interrupt Vector 0xFF"), "", this, &Z80Core::intVector, &Z80Core::setIntVector ),
    }} );
}

Z80Core::~Z80Core() {}

int Z80Core::getCpuReg( QString reg ) // Called by Mcu Monitor to get Integer values
{
    int value = -1;
    if( reg == "A") value = regA;
    if( reg == "B") value = regs[rB];
    if( reg == "C") value = regs[rC];
    if( reg == "D") value = regs[rD];
    if( reg == "E") value = regs[rE];
    if( reg == "H") value = regs[rH];
    if( reg == "L") value = regs[rL];
    if( reg == "XH") value = regs[rXH];
    if( reg == "XL") value = regs[rXL];
    if( reg == "YH") value = regs[rYH];
    if( reg == "YL") value = regs[rYL];
    if( reg == "I") value = regI;
    if( reg == "R") value = regR;
    if( reg == "AF") value = regPairs[rAF];
    if( reg == "BC") value = regPairs[rBC];
    if( reg == "DE") value = regPairs[rDE];
    if( reg == "HL") value = regPairs[rHL];
    if( reg == "IX") value = regPairs[rIX];
    if( reg == "IY") value = regPairs[rIY];
    if( reg == "AF'") value = regAAlt << 8 | regFAlt;
    if( reg == "BC'") value = regsAlt[rB] << 8 | regsAlt[rC];
    if( reg == "DE'") value = regsAlt[rD] << 8 | regsAlt[rE];
    if( reg == "HL'") value = regsAlt[rH] << 8 | regsAlt[rL];
    if( reg == "IR") value = regI << 8 | regR;
    if( reg == "SP") value = regSP;
    if( reg == "WZ") value = regWZ;

    if( reg == "TState_Int") value = sm_TStatesAfterInt;
    if( reg == "Op_Code") value = m_iReg;
    if( reg == "Address_Bus") value = sAO;
    if( reg == "Data_Bus_In") value = sDI;
    if( reg == "Data_Bus_Out") value = sDO;

    return value;
}

QString Z80Core::getStrReg( QString reg ) // Called by Mcu Monitor to get String values
{
    QString value = "";
    if( reg == "IFF1" ) value = IFF1 ? "enabled" : "disabled";
    if( reg == "IFF2" ) value = IFF2 ? "enabled" : "disabled";
    if( reg == "TState" ) value = QString::number(sm_TState) + " / " + QString::number(mc_TStates);
    if( reg == "MCycle" ) value = QString::number(sm_MCycle) + " / " + QString::number(mc_MCycles);
    if( reg == "M1_Type") switch(sm_M1CycleType) {
                          case tOpCodeFetch:  value = "OpCode fetch"; break;
                          case tInt:          value = "Int"; break;
                          case tNMI:          value = "NMI"; break;
                          case tHalt:         value = "HALT"; break;
                          case tSpecialReset: value = "Special reset"; break;
                          default:            value = "Don't know";
                          }
    if( reg == "Instruction") value = getStrInst();
    if( reg == "Bus_Op")  switch(mc_busOp) {
                          case oNone:     value = "None"; break;
                          case oM1:       value = "M1"; break;
                          case oIntAck:   value = "Interrupt acknowledge"; break;
                          case oMemRead:  value = "Memory read"; break;
                          case oMemWrite: value = "Memory write"; break;
                          case oIORead:   value = "IO read"; break;
                          case oIOWrite:  value = "IO write"; break;
                          default:        value = "Don't know";
                        }
    if( reg == "Bus_HighZ" ) value = highImpedanceBus ? "high Z" : "low Z";

    return value;
}

QString Z80Core::getStrInst()
{
    QString strInst;
    uint8_t mathOp;
    
    if( m_iSet == noPrefix ) {
        switch( m_iReg & 0xc0 ) {
            case 0x00: if( m_iReg == 0x00 ) strInst = "NOP";
                       if( m_iReg == 0x08 ) strInst = "EX AF,AF'";
                       if( m_iReg == 0x10 ) strInst = "DJNZ " + getStrReg8();
                       if( m_iReg == 0x18 ) strInst = "JR " + getStrReg8();
                       if( (m_iReg & 0x27) == 0x20 ) strInst = "JR " + getStrFlag( (m_iReg >> 3) & 0x03 ) + "," + getStrReg8();
                       if( (m_iReg & 0x0f) == 0x01 ) strInst = "LD " + getStrRegPair( (m_iReg >> 4) & 0x03 ) + "," + getStrReg16( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x0f) == 0x09 ) strInst = "ADD " + getStrRegPair( (m_iReg >> 4) & 0x03 ) + "," + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x2f) == 0x02 ) strInst = "LD (" + getStrRegPair( (m_iReg >> 4) & 0x01 ) + "),A";
                       if( (m_iReg & 0x2f) == 0x0a ) strInst = "LD A,(" + getStrRegPair( (m_iReg >> 4) & 0x01 ) + ")";
                       if( m_iReg == 0x22 ) strInst = "LD (" + getStrReg16() + ")," + getStrRegPair( (m_iReg >> 4) & 0x01 );
                       if( m_iReg == 0x2a ) strInst = "LD " + getStrRegPair( (m_iReg >> 4) & 0x01 ) + ",(" + getStrReg16() + ")";
                       if( m_iReg == 0x32 ) strInst = "LD (" + getStrReg16() + "),A";
                       if( m_iReg == 0x3a ) strInst = "LD A,(" + getStrReg16() + ")";
                       if( (m_iReg & 0x0f) == 0x03 ) strInst = "INC " + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x0f) == 0x0b ) strInst = "DEC " + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x07) == 0x04 ) strInst = "INC " + getStrReg( (m_iReg >> 3) & 0x07 );
                       if( (m_iReg & 0x07) == 0x05 ) strInst = "DEC " + getStrReg( (m_iReg >> 3) & 0x07 );
                       if( (m_iReg & 0x07) == 0x06 ) strInst = "LD " + getStrReg( (m_iReg >> 3) & 0x07 ) + "," + getStrReg8();
                       if( m_iReg == 0x07 ) strInst = "RLCA";
                       if( m_iReg == 0x0f ) strInst = "RRCA";
                       if( m_iReg == 0x17 ) strInst = "RLA";
                       if( m_iReg == 0x1f ) strInst = "RRA";
                       if( m_iReg == 0x27 ) strInst = "DAA";
                       if( m_iReg == 0x2f ) strInst = "CPL";
                       if( m_iReg == 0x37 ) strInst = "SCF";
                       if( m_iReg == 0x3f ) strInst = "CCF";
                       break;
            case 0x40: if( m_iReg != 0x76 ) strInst = "LD " + getStrReg( (m_iReg >> 3) & 0x07 ) + "," + getStrReg( m_iReg & 0x07 );
                       else                 strInst = "HALT";
                       break;
            case 0x80: mathOp = (m_iReg >> 3) & 0x07;
                       strInst = getStrMathOp( mathOp ) + " ";
                       if( mathOp == 0 || mathOp == 1 || mathOp == 3) strInst += "A,";
                       strInst += getStrReg( m_iReg & 0x07 );
                       break;
            case 0xc0: if( (m_iReg & 0x07) == 0x00 ) strInst = "RET " + getStrFlag( (m_iReg >> 3) & 0x07 );
                       if( (m_iReg & 0x0f) == 0x01 ) strInst = "POP " + getStrRegPair( (m_iReg >> 4) & 0x03, true );
                       if( m_iReg == 0xc9 ) strInst = "RET";
                       if( m_iReg == 0xd9 ) strInst = "EXX";
                       if( m_iReg == 0xe9 ) strInst = "JP (" + getStrRegPair( 2 ) + ")";
                       if( m_iReg == 0xf9 ) strInst = "LD SP," + getStrRegPair( 2 );
                       if( (m_iReg & 0x07) == 0x02 ) strInst = "JP " + getStrFlag( (m_iReg >> 3) & 0x07 ) + "," + getStrReg16();
                       if( m_iReg == 0xc3 ) strInst = "JP " + getStrReg16();
                       if( m_iReg == 0xcb ) strInst = "PREFIX CB";
                       if( m_iReg == 0xd3 ) strInst = "OUT (" + getStrReg8() + "),A";
                       if( m_iReg == 0xdb ) strInst = "IN A,(" + getStrReg8() + ")";
                       if( m_iReg == 0xe3 ) strInst = "EX (SP)," + getStrRegPair( 2 );
                       if( m_iReg == 0xeb ) strInst = "EX DE,HL";
                       if( m_iReg == 0xf3 ) strInst = "DI";
                       if( m_iReg == 0xfb ) strInst = "EI";
                       if( (m_iReg & 0x07) == 0x04 ) strInst = "CALL " + getStrFlag( (m_iReg >> 3) & 0x07 ) + "," + getStrReg16();
                       if( (m_iReg & 0x0f) == 0x05 ) strInst = "PUSH " + getStrRegPair( (m_iReg >> 4) & 0x03, true );
                       if( m_iReg == 0xcd ) strInst = "CALL " + getStrReg16();
                       if( m_iReg == 0xdd ) strInst = "PREFIX IX";
                       if( m_iReg == 0xed ) strInst = "PREFIX ED";
                       if( m_iReg == 0xfd ) strInst = "PREFIX IY";
                       if( (m_iReg & 0x07) == 0x06 ) strInst = getStrMathOp( (m_iReg >> 3) & 0x07 ) + " " + getStrReg8();
                       if( (m_iReg & 0x07) == 0x07 ) strInst = "RST " + QString::number( m_iReg & 0x38 );
                       break;
        }
    }

    if( m_iSet == prefixCB ) {
        switch( m_iReg & 0xc0 ) {
            case 0x00: switch( m_iReg & 0xf8 ) {
                           case 0x00: strInst = "RLC "; break;
                           case 0x08: strInst = "RRC "; break;
                           case 0x10: strInst = "RL "; break;
                           case 0x18: strInst = "RR "; break;
                           case 0x20: strInst = "SLA "; break;
                           case 0x28: strInst = "SRA "; break;
                           case 0x30: strInst = "SLL "; break;
                           case 0x38: strInst = "SRL "; break;
                       }
                       if( XYState == rHL ) strInst += getStrReg( m_iReg & 0x07 );
                       else                 strInst = "LD " + getStrReg( m_iReg & 0x07 ) + "," + strInst + getStrReg( 6 );
                       break;
            case 0x40: strInst = "BIT " + QString::number( (m_iReg >> 3) & 0x07 ) + "," + getStrReg( m_iReg & 0x07 ); break;
            case 0x80: if( XYState == rHL ) strInst = "RES " + QString::number( (m_iReg >> 3) & 0x07 ) + "," + getStrReg( m_iReg & 0x07 );
                       else                 strInst = "LD " + getStrReg( m_iReg & 0x07 ) + ",RES " + QString::number( (m_iReg >> 3) & 0x07 ) + getStrReg( 6 );
                       break;
            case 0xc0: if( XYState == rHL ) strInst = "SET " + QString::number( (m_iReg >> 3) & 0x07 ) + "," + getStrReg( m_iReg & 0x07 );
                       else                 strInst = "LD " + getStrReg( m_iReg & 0x07 ) + ",SET " + QString::number( (m_iReg >> 3) & 0x07 ) + getStrReg( 6 );
                       break;
        }
    }

    if( m_iSet == prefixED ) {
        switch( m_iReg & 0xc0 ) {
            case 0x40: if( m_iReg == 0x70 ) { strInst = "IN (C)"; break; }
                       if( (m_iReg & 0x07) == 0x00 ) strInst = "IN " + getStrReg( (m_iReg >> 3) & 0x07 ) + "(C)";
                       if( m_iReg == 0x71 ) { strInst = (m_cmos == false) ? "OUT (C),0" : "OUT (C),FF"; break; }
                       if( (m_iReg & 0x07) == 0x01 ) strInst = "OUT (C)," + getStrReg( (m_iReg >> 3) & 0x07 );
                       if( (m_iReg & 0x0f) == 0x02 ) strInst = "SBC HL," + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x0f) == 0x0a ) strInst = "ADC HL," + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x0f) == 0x03 ) strInst = "LD (" + getStrReg16( 5 ) + ")," + getStrRegPair( (m_iReg >> 4) & 0x03 );
                       if( (m_iReg & 0x0f) == 0x0b ) strInst = "LD " + getStrRegPair( (m_iReg >> 4) & 0x03 ) + ",(" + getStrReg16( 5 ) + ")";
                       if( (m_iReg & 0x07) == 0x04 ) strInst = "NEG";
                       if( m_iReg == 0x4d ) { strInst = "RETI"; break; }
                       if( (m_iReg & 0x07) == 0x05 ) strInst = "RETN";
                       if( (m_iReg & 0x1f) == 0x06 ) strInst = "IM 0";
                       if( (m_iReg & 0x1f) == 0x0e ) strInst = "IM 0/1";
                       if( (m_iReg & 0x1f) == 0x16 ) strInst = "IM 1";
                       if( (m_iReg & 0x1f) == 0x1e ) strInst = "IM 2";
                       if( m_iReg == 0x47 ) strInst = "LD I,A";
                       if( m_iReg == 0x4f ) strInst = "LD R,A";
                       if( m_iReg == 0x57 ) strInst = "LD A,I";
                       if( m_iReg == 0x5f ) strInst = "LD A,R";
                       if( m_iReg == 0x67 ) strInst = "RRD";
                       if( m_iReg == 0x6f ) strInst = "RLD";
                       break;
            case 0x80: if( m_iReg == 0xa0 ) { strInst = "LDI"; break; }
                       if( m_iReg == 0xa1 ) { strInst = "CPI"; break; }
                       if( m_iReg == 0xa2 ) { strInst = "INI"; break; }
                       if( m_iReg == 0xa3 ) { strInst = "OUTI"; break; }
                       if( m_iReg == 0xa8 ) { strInst = "LDD"; break; }
                       if( m_iReg == 0xa9 ) { strInst = "CPD"; break; }
                       if( m_iReg == 0xaa ) { strInst = "IND"; break; }
                       if( m_iReg == 0xab ) { strInst = "OUTD"; break; }
                       if( m_iReg == 0xb0 ) { strInst = "LDIR"; break; }
                       if( m_iReg == 0xb1 ) { strInst = "CPIR"; break; }
                       if( m_iReg == 0xb2 ) { strInst = "INIR"; break; }
                       if( m_iReg == 0xb3 ) { strInst = "OTIR"; break; }
                       if( m_iReg == 0xb8 ) { strInst = "LDDR"; break; }
                       if( m_iReg == 0xb9 ) { strInst = "CPDR"; break; }
                       if( m_iReg == 0xba ) { strInst = "INDR"; break; }
                       if( m_iReg == 0xbb ) { strInst = "OTDR"; break; }
            case 0x00:
            case 0xc0: strInst = "Illegal prefix ED instruction"; break;
        }
    }
        
    return strInst;
}

QString Z80Core::getStrMathOp( uint8_t reg )
{
    QString strMathOp;
    
    switch( reg ) {
        case 0 : strMathOp = "ADD"; break;
        case 1 : strMathOp = "ADC"; break;
        case 2 : strMathOp = "SUB"; break;
        case 3 : strMathOp = "SBC"; break;
        case 4 : strMathOp = "AND"; break;
        case 5 : strMathOp = "XOR"; break;
        case 6 : strMathOp = "OR"; break;
        case 7 : strMathOp = "CP"; break;
    }
    return strMathOp;
}

QString Z80Core::getStrFlag( uint8_t reg )
{
    QString strFlag;
    
    switch( reg ) {
        case 0: strFlag = "NZ"; break;
        case 1: strFlag = "Z"; break;
        case 2: strFlag = "NC"; break;
        case 3: strFlag = "C"; break;
        case 4: strFlag = "PO"; break;
        case 5: strFlag = "PE"; break;
        case 6: strFlag = "P"; break;
        case 7: strFlag = "M"; break;
        default: strFlag = "?";
    }
    return strFlag;
}

QString Z80Core::getStrReg( uint8_t reg )
{
    QString strReg;
    static int8_t offset;
    
    switch( reg ) {
        case 0: strReg = "B"; break;
        case 1: strReg = "C"; break;
        case 2: strReg = "D"; break;
        case 3: strReg = "E"; break;
        case 4: strReg = XYState == rIX ? "XH" : ( XYState == rIY ? "YH" : "H" ); break;
        case 5: strReg = XYState == rIX ? "XL" : ( XYState == rIY ? "YL" : "L" ); break;
        case 6: if( XYState == rHL ) strReg = "(HL)"; break; //?? tady se musí doplnit ještě IX+d a IY+d
                if( XYState == rIX ) strReg = "(IX";
                if (XYState == rIY ) strReg = "(IY";
                switch( sm_MCycle ) {
                    case 1:  if ( sm_TState >= 3) { strReg += "+??"; break; }
                    default: offset = static_cast<int>( regWZ ) - static_cast<int>( regPairs[XYState] );
                             strReg += (offset >= 0) ? "+" + QString::number( offset, 16 ).toUpper() : "-" + QString::number( -offset, 16 ).toUpper(); break;
                    case 6:  strReg += "+??";
                }
                strReg += ")";
                break;
        case 7: strReg = "A"; break;
        default: strReg = "?";
    }
    return strReg;
}

QString Z80Core::getStrRegPair( uint8_t reg, bool set )
{
    QString strRegPair;
    
    switch( reg ) {
        case 0: strRegPair = "BC"; break;
        case 1: strRegPair = "DE"; break;
        case 2: strRegPair = XYState == rIX ? "IX" : ( XYState == rIY ? "IY" : "HL" ); break;
        case 3: if( set ) strRegPair = "AF";
                else      strRegPair = "SP";
                break;
        default: strRegPair = "??";
    }
    return strRegPair;
}

QString Z80Core::getStrReg8()
{
    QString strReg8;
    
    switch( sm_MCycle ) {
        case 1:  if ( sm_TState >= 3) { strReg8 = "??"; break; }
        default: strReg8 = QString::number( sDI, 16 ).toUpper(); break;
        case 2:  strReg8 = "??";
    }
    return strReg8;
}

QString Z80Core::getStrReg16( uint8_t reg )
{
    QString strReg16;
    static uint16_t value = 0;
    
    switch( reg ) {
        case 0: value = regPairs[rBC]; break;
        case 1: value = regPairs[rDE]; break;
        case 2: value = XYState == rIX ? regPairs[rIX] : ( XYState == rIY ? regPairs[rIY] : regPairs[rHL] ); break;
        case 3: value = regSP; break;
        case 4: value = regWZ; break;
        case 5: if( sm_MCycle == 3 ) value = regWZ;
                if( sm_MCycle > 3 ) value = regWZ - 1;
    }

    switch( sm_MCycle ) {
        case 1:  if ( sm_TState >= 3) { strReg16 = "????"; break; }
        default: strReg16 = QString::number( value, 16 ).toUpper(); break;
        case 2:  strReg16 = "????"; break;
        case 3:  strReg16 = "??" + QString::number( value & 0xff, 16 ).toUpper(); break;
    }

    return strReg16;
}

void Z80Core::stamp()
{
    m_mreqPin->setOutStatFast( true );
    m_iorqPin->setOutStatFast( true );
    m_rdPin->setOutStatFast( true );
    m_wrPin->setOutStatFast( true );
    m_rfshPin->setOutStatFast( true );

    m_m1Pin->setPinMode( output );
    m_m1Pin->setOutStatFast( true );
    m_haltPin->setPinMode( output );
    m_haltPin->setOutStatFast( true );
    m_waitPin->setPinMode( input );
    m_intPin->setPinMode( input );
    m_nmiPin->setPinMode( input );
    m_resetPin->setPinMode( input );
    m_busreqPin->setPinMode( input );
    m_busacPin->setPinMode( output );
    m_busacPin->setOutStatFast( true );

    // ?? followed four lines must be in stamp(), it defines reset behaviour and must not be reset in reset() method
    normalReset = false;                /// reset flag normalReset
    specialReset = false;               /// reset flag specialReset
    rstCount = 0;                       /// reset TState counter for reset
    m_nextClock = true; /// ???
}

void Z80Core::runEvent()
{
    if( !m_nextClock ) risingEdgeDelayed();  // Rising edge
    else               fallingEdgeDelayed(); // Falling edge
}

void Z80Core::reset()
{
    Simulator::self()->cancelEvents( this );

    releaseBus( false );

    // Reset Z80Core registers
    regA = 0xFF;
    regF = 0xFF;
    regAAlt = 0xFF;
    regFAlt = 0xFF;
    regI = 0;
    regR = 0;
    regWZ = 0;
    regSP = 0xFFFF;
    m_PC = 0;
    IFF1 = false;
    IFF2 = false;
    regF.clearChanged();

    // Reset Z80Core MCode variables
    mc_TStates = 4;
    mc_MCycles = 1;
    mc_StateMachine = sNone;
    mc_prefix = noPrefix;
    mc_busOp = oNone;

    // Reset Z80Core variables
    m_iSet = noPrefix;
    XYState = rHL;
    m_lastBusOp = oNone;
    m_iReg = 0x00;
    intMode = 0;
    highImpedanceBus = true;

    // Reset Z80Core main state machine variables
    sm_TState = 0;
    sm_lastTState = true;
    sm_TStatesAfterInt = Z80CORE_MAX_T_INT;
    sm_MCycle = 1;
    sm_PreXYMCycle = 1;
    sm_M1CycleType = tOpCodeFetch;
    sm_waitTState = false;
    sm_autoWait = 0;
    sm_waitTState = false;

    // Reset Z80Core sampled bus signals
    NMIFF = false;
    sLastNMI = false;
    sNMI = false;
    sInt = false;
    sWait = false;
    sBusReq = false;
    sBusAck = false;
    sDI = 0x00;
    sDO = 0x00;
    sAO = 0x0000;
}

void Z80Core::extClock( bool clkState ) // External Clock
{
    if( clkState == m_nextClock ) runStep();
}

void Z80Core::runStep()
{
    if( m_nextClock ) clkRisingEdge();
    else              clkFallingEdge();
    m_nextClock = !m_nextClock;
}

void Z80Core::clkRisingEdge() // Execution of instruction and sampling bus signal at clock rising edge
{
    //  Reset is accepted after three TStates
    if( !m_resetPin->getInpState() ){               // if reset input is active, then count TStates until it reachs value 2, then set flag normalReset
        if( !normalReset ){                         // the flag normalReset is not set yet
            if( rstCount == 2 ) normalReset = true; // set flag normalReset
            else                rstCount++;         // increase TStates counter for reset
        }
    }else {                                         // if reset input is not active, then count TStates for reset down until it reach value 0, then reset CPU
        if( normalReset ){                          // the flag normalReset is still set
            if( rstCount > 0 ) rstCount--;          // decrease TStates counter for reset
            else {reset(); normalReset=false;}      // when TStates counter for reset reach 0 reset CPU
        }
        else rstCount = 0;                          // restart TStates counter for reset when the reset pulse is shorter than 3 TStates
    }
    // Increase counter TStates after interrupt (it is limited to Z80CORE_MAX_T_INT)
    if( sm_TStatesAfterInt < Z80CORE_MAX_T_INT ) sm_TStatesAfterInt++;

    // Increase TStaste and execute instructions only when the bus is used by cpu, otherwise cpu is halted
    if( !normalReset && ( !sBusReq || !sBusAck ) )
    {
        sBusAck = false;                             // finish bus request
        m_lastBusOp = mc_busOp;                      // store previous bus op.

        // Execute instruction at rising edge of TState 1
        if( sm_lastTState && !sBusReq )             // is executed at last TState, but after bus request and release when it is inserted
            runMCode();                             // execute instruction - this function change number of machine cycles and TStates
        nextTState();                               // go to the next TState

        if( sm_TState == 3 && sm_MCycle == 1 ) opCodeFetch(); // Op Code Fetch at rising edge of TState 3

        // Op Code Fetch at rising edge of TState 4 and machine cycle 7 in case of prefix CB
        // This is simple version of function opCodeFetch()
        // Op Code is read from sDI register instead of data bus, because the machine cycle 7 is memory read cycle and not op code fetch cycle
        // and the data are valid at different clock edge.
        if( sm_TState == 4 && sm_MCycle == 7 && mc_prefix == prefixCB )
        {
            m_iReg = sDI;                                   // reading opcode from data bus
            m_PC++;                                        // increase program counter PC
            m_iSet = prefixCB;                              // switch instruction set to prefix CB
            mc_MCycles = sTableMCycles[prefixCB][m_iReg];   // set number of machine cycles for instruction
        }
    }
    sLastNMI = sNMI;
    sNMI    = !m_nmiPin->getInpState(); // Sampling of bus signals NMI, INT and BUSREQ
    if ( !sLastNMI && sNMI )
        NMIFF = true;
    sInt    = !m_intPin->getInpState();
    sBusReq = !m_busreqPin->getInpState(); /// At Rising edge latst T State ??? - I guess it doesn't matter

    // RisingEdgeDelayed
    Simulator::self()->addEvent( m_delay, this );
}

void Z80Core::clkFallingEdge() // Sampling bus signal at clock falling edge
{
    sWait = !m_waitPin->getInpState(); // Sampling wait signal

    /// Done at fallingEdgeDelayed(), is OK???
    // Sampling data bus for Memory read and Input/Output read cycle
    /*if ( (sm_TState == 3) && ( (mc_busOp == oMemRead) || (mc_busOp == oIORead) ) && (sBusAck == false) ) {
        sDI = readDataBus();
    }*/

    // FallingEdgeDelayed
    Simulator::self()->addEvent( m_delay, this );
}

// Increasing TState, if it is last TState then TState is reset and MCycle is increased
// It also contains handling of bus request, halt, interrupt and maskable interrupt
void Z80Core::nextTState()
{
    if( sm_TState != 2 || sWait == false ) // Hold in TState 2 when Wait signal is actived
    {
        if( sm_lastTState ) // If the TState is the last state then it is necessary to increase machine cycle
        {
            if( mc_StateMachine == sHalt) // Set Machine cycle to halt machine cycle
            {
                sm_M1CycleType = tHalt;                             // machine cycle halt
                mc_StateMachine = sNone;                            // reset flag
            }
            if( sBusReq ) sBusAck = true; // The bus is release when it is required by signal BusReq
            else {
                sm_TState = 1;                                      // reset TState to first state

                if( mc_StateMachine == sXYOffsetFetch ) // set machine cycle to IX/IY Address offset fetch cycle (Machine cycle number 6)
                {
                    sm_PreXYMCycle = sm_MCycle;                     // store actual machine cycle to be able to return (cycles are inserted)
                    sm_MCycle = 6;                                  // address offset fetch cycle is cycle number 6
                    mc_StateMachine = sNone;                        // reset flag
                }
                else if( sm_MCycle == 7 || mc_StateMachine == sXYFetchFinish )   // last IX/IY fetch cycle is 7, but it can be finish earlier by flag
                {
                    sm_MCycle = sm_PreXYMCycle + 1;             // restore machine cycle to cycle before IX/IY Address offset fetch cycle plus one
                    mc_StateMachine = sNone;                    // reset flag
                }
                else if( sm_MCycle == mc_MCycles )              // Last Machine cycle of instruction
                {
                    sm_MCycle = 1;                              // set machine cycle number 1
                    mc_busOp = oM1;                             // bus operaton Op Code Fetch
                    if( sm_M1CycleType != tHalt ) sm_M1CycleType = tOpCodeFetch; // machine cycle Op Code Fetch only when the previous Machine cycle is not halt machine cycle

                    if( specialReset ) {             // if the special reset is required then the next machine cycle is special reset cycle
                        sm_M1CycleType = tSpecialReset;
                        specialReset = false;               // reset special reset flag
                    }
                    if( NMIFF && mc_prefix == noPrefix ) // Non maskable interrupt only when instruction is completed with all prefixes
                    {
                        NMIFF = false;
                        sm_M1CycleType = tNMI;              // machine cycle nonmaskable interrupt, bus op. Op Code Fetch
                        IFF1 = false;                       // disable interrupt
                    }
                    else if( sInt && IFF1 && mc_prefix == noPrefix )// Interrupt only when instruction is completed with all prefixes and interrupt is enabled
                    {
                        mc_busOp = oIntAck;             // bus op. interrupt
                        sm_M1CycleType = tInt;          // machine cycle interrupt
                        sm_autoWait = 1;                // one wait state during INT M1 cycle TState 1
                        IFF1 = false;                   // disable interrupt
                        IFF2 = false;
                        sm_TStatesAfterInt = 0;         // reset TStates after interrupt counter
                    }
                }
                else sm_MCycle++;                            // increase machine cycle
            }
        } else {
            sm_waitTState = (sm_autoWait > 0);                      // increase TState when wait state is not required
            if( sm_waitTState == false )
            {
                sm_TState++;                                        // increase TState
                // I/O operations and interrupt require one T2 wait state extra (it is possible to disable it by variable m_ioWait)
                if( m_ioWait && sm_TState == 2 && (mc_busOp == oIORead || mc_busOp == oIOWrite || mc_busOp == oIntAck) )
                    sm_autoWait = 1;                                // set number of inserted wait states to 1
                if( sm_TState == 2 && !m_resetPin->getInpState() )  //  when the RESET signal is set at rising edge of TState 2 the special reset will be executed
                    specialReset = true;                            // set special reset flag
            }
            else sm_autoWait--;                                      // decrease number of inserted wait states
        }
        sm_lastTState = (sm_TState == mc_TStates);                  // set flag of the last TState
    }
    else sm_waitTState = true;                                      // when the TState = 2 and Wait is active it is a wait TState
}

void Z80Core::risingEdgeDelayed()
{
    // When bus is requested by other device by BUSRQ signal then signal BUSACK is set and bus is set to high impedance
    // or when normal reset is requested by RESET signal the bus is set to high impedance
    if( ( sBusAck || normalReset ) && !highImpedanceBus )  /// At T1 after BUSREQ ???
    {
        /// confusion betwen sBusAck and sBusReq
        m_busacPin->setOutStatFast( false );               // set BUSACK
        releaseBus( true );
    }
    // Setting bus only when not requested by BUSRQ and acknowledged by signal BUSACK or normal reset is requested by RESET signal
    if( !sBusAck && !normalReset  )                      /// At T1 after BUSREQ ???
    {
        if( highImpedanceBus ) releaseBus( false ); // If bus is in high impedance then set bus to low impedance

        switch( sm_TState )// Setting bus at clock rising edge of TState 1 (it might repeat when wait states are inserted during interrupt)
        {
        case 1: if( !sm_waitTState  )
                {
                    m_addrPort->setOutStatFast( sAO );
                    if( m_lastBusOp == oMemWrite || m_lastBusOp == oIOWrite ) m_dataPort->setPinMode( input );// If previous bus op. was write data then release data bus
                    if( mc_busOp    == oM1 || mc_busOp    == oIntAck )  m_m1Pin->setOutStatFast( false );// If current bus op. is Fetch (machine cycle 1) then set M1
                    if( m_lastBusOp == oM1 || m_lastBusOp == oIntAck  ) m_rfshPin->setOutStatFast( true );// If previous bus op. was Fetch (machine cycle 1) then signal RFSH is reset
                }
                break;
        case 2: if( sm_waitTState == false )// Setting bus at clock rising edge of TState 2 (it might repeat when wait states are inserted)
                {
                    if( mc_busOp == oIORead ) {              // If current bus op. is read I/O then set IORQ and RD
                        m_iorqPin->setOutStatFast( false );
                        m_rdPin->setOutStatFast( false );
                    }
                    if( mc_busOp == oIOWrite ) {            // If current bus op. is write I/O then set IORQ and WR
                        m_iorqPin->setOutStatFast( false );
                        m_wrPin->setOutStatFast( false );
                    }
                }
                break;
        case 3: if( mc_busOp == oM1 || mc_busOp == oIntAck ) // If current bus op. is Fetch (machine cycle 1) then reset M1, set RFSH and update address bus
                {
                    m_m1Pin->setOutStatFast( true );
                    m_rfshPin->setOutStatFast( false );

                    /// address value should be already available
                    m_addrPort->setOutStatFast( (regI << 8) | regR );
                    regR = ( (regR + 1) & 0x7f ) + ( regR & 0x80 );
                }
                if( mc_busOp == oM1 ) {// It current bus op. is Fetch without interrupt then reset MREQ and RD
                    m_mreqPin->setOutStatFast( true );
                    m_rdPin->setOutStatFast( true );
                }
                if( mc_busOp == oIntAck ) m_iorqPin->setOutStatFast( true );// If current bus op. is Fetch with interrupt then reset IORQ
                break;
                // Setting bus at clock rising edge of TState 5 in case of longer machine cycle than 4 TStates
        case 5: if( mc_busOp == oM1 || mc_busOp == oIntAck ) // If bus op. is Fetch (machine cycle 1) then address bus is restored and reset RFSH
                {
                    m_addrPort->setOutStatFast( sAO );
                    m_rfshPin->setOutStatFast( true );
                }
        }
    }
}

void Z80Core::fallingEdgeDelayed()
{
    if( sBusAck == false ) { // Setting bus only when bus is not requested by signal BUSRQ and acknowledged by signal BUSACK
        switch( sm_TState ) {
                // Setting bus at clock rising edge of TState 1 (it might repeat when wait states are inserted during interrupt)
        case 1: if( sm_waitTState == false )
                {
                    if( mc_busOp == oM1 ) {// If  current bus op. is Fetch without interrupt (machine cycle 1) then set MREQ and M1
                        m_mreqPin->setOutStatFast( false );
                        m_rdPin->setOutStatFast( false );   /// ERROR ???
                    }
                    if( mc_busOp == oMemRead ) {              // If  current bus op. is read  memory then set MREQ and RD
                        m_mreqPin->setOutStatFast( false );
                        m_rdPin->setOutStatFast( false );
                    }
                    if( mc_busOp == oMemWrite) m_mreqPin->setOutStatFast( false ); // If  current bus op. is write memory then set MREQ
                    if( mc_busOp == oMemWrite || mc_busOp == oIOWrite ) {// If current bus op. is write memory or I/O then set data bus
                        m_dataPort->setPinMode( output );
                        m_dataPort->setOutStatFast( sDO );
                    }
                }
                break;
        case 2: if( sm_waitTState == false ) {// Setting bus at clock rising edge of TState 2 (it might repeat when wait states are inserted)
                    // If  current bus op. is Fetch with interrupt (machine cycle 1) then set IORQ
                    // (There is a difference in TStates numbering from datasheet - one automatically inserted wait states is T1 wait)
                    if (mc_busOp == oIntAck) m_iorqPin->setOutStatFast( false );
                    if (mc_busOp == oMemWrite) m_wrPin->setOutStatFast( false ); // If  current bus op. is write memory then set WR
                }
                break;
        case 3: if( mc_busOp == oMemRead || mc_busOp == oIORead )// Sampling data bus for Memory read and Input/Output read cycle
                    sDI = m_dataPort->getInpState();

            /// If  current bus op. is Fetch (machine cycle 1) then reset MREQ
            /// reset or set???
                if( mc_busOp == oM1 || mc_busOp == oIntAck )
                    m_mreqPin->setOutStatFast( false );

                if( mc_busOp == oMemRead ) {                 // If  current bus op. is read memory then reset MREQ and RD
                    m_mreqPin->setOutStatFast( true );
                    m_rdPin->setOutStatFast( true );
                }
                if( mc_busOp == oMemWrite ) {               // If  current bus op. is write memory then reset MREQ and WR
                    m_mreqPin->setOutStatFast( true );
                    m_wrPin->setOutStatFast( true );
                }
                if( mc_busOp == oIORead ) {                 // If  current bus op. is read I/O then reset IORQ and RD
                    m_iorqPin->setOutStatFast( true );
                    m_rdPin->setOutStatFast( true );
                }
                if( mc_busOp == oIOWrite ) {                // If  current bus op. is write I/O then reset IORQ and WR
                    m_iorqPin->setOutStatFast( true );
                    m_wrPin->setOutStatFast( true );
                }
                break;
        case 4: if( mc_busOp == oM1 || mc_busOp == oIntAck ) m_mreqPin->setOutStatFast( true ); // If current bus op. is Fetch (machine cycle 1) then reset MREQ
            /// create Halt variable, state or whatever
                if( m_iReg == 0x76 && m_iSet == noPrefix )  m_haltPin->setOutStatFast( false ); // If  current instruction is HALT then set HALT
                if( sNMI || ( sInt && IFF1 ) )              m_haltPin->setOutStatFast( true ); // If interrupt is requested and enabled then reset HALT
        }
    }/// confusion betwen sBusAck and sBusReq:
    else if( !sBusReq ) m_busacPin->setOutStatFast( true ); // If bus is not requested any more by BUSRQ signal then signal BUSACK is reset
}

void Z80Core::releaseBus( bool rel )
{
    pinMode_t mode = rel ? input : output;

    m_mreqPin->setPinMode( mode );                          // MREQ to low impedance
    m_iorqPin->setPinMode( mode );                          // IORQ to low impedance
    m_rdPin->setPinMode( mode );                            // RD   to low impedance
    m_wrPin->setPinMode( mode );                            // WR   to low impedance
    m_rfshPin->setPinMode( mode );                          // RFSH to low impedance

    //for( int i=0; i<16; ++i ) m_outPin[i]->setStateZ(  mode ); // set address bus to low impedance
    m_addrPort->setPinMode( mode );
    //releaseDataBus();                                       // set data bus to high impedance
    m_dataPort->setPinMode( input );

    highImpedanceBus = rel;                                 // reset flag that bus is in high impedance
}

// Read instruction to instruction register. The source of instruction depends on type of machine cycle 1. It can be read from data bus or instruction NOP or RST 38H.
// The instruction set is switched according to previous prefix instructions.
// The number of machine cycles and TStates for machine cycle 1 is set for the instruction.
// The program counter is increased in case of not handling interrupt or halt.
void Z80Core::opCodeFetch()
{
    switch( sm_M1CycleType ) // Fetching opcode
    {
        case tOpCodeFetch: m_iReg = m_dataPort->getInpState();   // reading opcode from data bus
            m_PC++;                                            // increase program counter PC

            switch( mc_prefix ){ // Handle prefixes
                case noPrefix: XYState = rHL; m_iSet = noPrefix; break; // no prefix - register pair HL and basic instruction set
                case prefixCB:                m_iSet = prefixCB; break; // prefix CB - no change register pair and instruction set CB
                case prefixED: XYState = rHL; m_iSet = prefixED; break; // prefix ED - register pair HL and instruction set ED
                case prefixIX: XYState = rIX; m_iSet = noPrefix; break; // prefix IX - register pair IX and basic instruction set
                case prefixIY: XYState = rIY; m_iSet = noPrefix; break; // prefix IY - register pair IY and basic instruction set
            }
            // Set number of machine cycles and TStates for instruction
            mc_MCycles = sTableMCycles[m_iSet][m_iReg];
            mc_TStates = sTableM1TStates[m_iSet][m_iReg];
            break;
                            // Fetching opcode for Interrupt (IM0, IM1 and IM2)
        case tInt:
            switch( intMode ){
                case 0:  m_iReg = m_dataPort->getInpState(); break; // from data bus
                case 1:  m_iReg = 0xFF; break;          // RST 38H
                case 2:  m_iReg = 0x00;                 // NOP for IM2 has 5 machine cycles and 5 TStates
                         sDI = m_dataPort->getInpState();  // Read interrupt vector
                         mc_MCycles = 5;
                         mc_TStates = 5;
                         break;
                default: m_iReg = 0x00; break;
            }
            // Set number of machine cycles and TStates for instruction except IM2
            if (intMode != 2) {
                mc_MCycles = sTableMCycles[m_iSet][m_iReg];
                mc_TStates = sTableM1TStates[m_iSet][m_iReg];
            }
            break;
                            // Fetching opcode for NMI
        case tNMI: m_iReg = 0x00; mc_MCycles = 3; mc_TStates = 5; break; // NOP for NMI has 3 machine cycles and 5 TStates

                            // Fetching opcode for special reset - set program counter PC to zero and execute instruction NOP
        case tSpecialReset: m_PC = 0;                                  // set program counter PC to zero
                            m_iSet = noPrefix;                          // no prefix - register pair HL and basic instruction set

                            // Intentionally missing break before case Fetching opcode for Halt
        case tHalt:         m_iReg = 0x00;                              // NOP
                            mc_MCycles = sTableMCycles[m_iSet][m_iReg];
                            mc_TStates = sTableM1TStates[m_iSet][m_iReg];
    }
}

// execution
// It is called at rising edge of last TState of every machine cycle - the internal CPU timing is not exact
void Z80Core::runMCode()
{
    mc_TStates = 3;                                             // default number of TStates is 3
    sAO = m_PC;                                                   // default value for address bus is program counter PC
    mc_prefix = noPrefix;                                       // default value for prefixes is no prefix
    regF.clearChanged();
//    lastRegFChanged = regFChanged;                              // store flag of changing register F by previous instruction - it is needed by instructions SCF and CCF
//    regFChanged = false;                                        // register F is not changed before instruction execution

    if (m_iSet == noPrefix) // Execute instructions without prefix
        switch(m_iReg) {
                       // NOP is used also at interrupt machine cycles
                       // When INT is actived and interrupt mode is set to 2 the interrupt vector is read from data bus and corresponding function is called
            case 0x00: if( sm_M1CycleType == tInt && intMode == 2 ) int_im2();                          // NOP
                       if( sm_M1CycleType == tNMI )                 rst( 0x66 ); // When NMI is actived the function at address 0x0066 is called
                       break;
            case 0x01: ld_rr_imm( regPairs[rBC] ); break;                                               // LD BC,NN
            case 0x02: ld_indir_r( regPairs[rBC], regA ); break;                                        // LD (BC),A
            case 0x03: regPairs[rBC]++; break;                                                          // INC BC
            case 0x04: mathOp< iInc >( regs[rB] ); break;                                               // INC B
            case 0X05: mathOp< iDec >( regs[rB] ); break;                                               // DEC B
            case 0x06: ld_r_imm(regs[rB]); break;                                                       // LD B,N
            case 0x07: shiftOp< iRlc, false >( regA ); break;                                           // RLCA
            case 0x08: ex_rr_rr( regPairs[rAF], regAAlt, regFAlt ); break;                              // EX AF,AF'
            case 0x09: mathOp16< iAdd >( regPairs[XYState], regPairs[rBC] ); break;                     // ADD HL,BC  ADD IX,BC  ADD IY,BC
            case 0x0A: ld_r_indir(regA, regPairs[rBC]); break;                                          // LD A,(BC)
            case 0x0B: regPairs[rBC]--; break;                                                          // DEC BC
            case 0x0C: mathOp< iInc >(regs[rC]); break;                                                 // INC C
            case 0X0D: mathOp< iDec >(regs[rC]); break;                                                 // DEC C
            case 0x0E: ld_r_imm(regs[rC]); break;                                                       // LD C,N
            case 0x0F: shiftOp< iRrc, false >( regA ); break;                                           // RRCA
            case 0x10: { if (sm_MCycle == 1) regs[rB]--; } jr( regs[rB] != 0 ); break;                  // DJNZ N
            case 0x11: ld_rr_imm( regPairs[rDE] ); break;                                               // LD DE,NN
            case 0x12: ld_indir_r( regPairs[rDE], regA ); break;                                        // LD (DE),A
            case 0x13: regPairs[rDE]++; break;                                                          // INC DE
            case 0x14: mathOp< iInc >(regs[rD]); break;                                                 // INC D
            case 0X15: mathOp< iDec >(regs[rD]); break;                                                 // DEC D
            case 0x16: ld_r_imm(regs[rD]); break;                                                       // LD D,N
            case 0x17: shiftOp< iRl, false >( regA ); break;                                            // RLA
            case 0x18: jr(true); break;                                                                 // JR N
            case 0x19: mathOp16< iAdd >( regPairs[XYState], regPairs[rDE] ); break;                     // ADD HL,DE  ADD IX,DE  ADD IY,DE
            case 0x1A: ld_r_indir( regA, regPairs[rDE] ); break;                                        // LD A,(DE)
            case 0x1B: regPairs[rDE]--; break;                                                          // DEC DE
            case 0x1C: mathOp< iInc >(regs[rE]); break;                                                 // INC E
            case 0X1D: mathOp< iDec >(regs[rE]); break;                                                 // DEC E
            case 0x1E: ld_r_imm(regs[rE]); break;                                                       // LD E,N
            case 0x1F: shiftOp< iRr, false >( regA ); break;                                            // RRA
            case 0x20: jr( !regF.getZ() ); break;                                                       // JR NZ,N
            case 0x21: ld_rr_imm( regPairs[XYState] ); break;                                           // LD HL,NN  LD IX,NN  LD IY,NN
            case 0x22: ld_mem_rr( regPairs[XYState] ); break;                                           // LD (NN),HL  LD (NN),IX  LD (NN),IY
            case 0x23: regPairs[XYState]++; break;                                                      // INC HL  INC IX  INC IY
            case 0x24: mathOp< iInc >(regs[XYState]); break;                                            // INC H  INC XH  INC YH
            case 0X25: mathOp< iDec >(regs[XYState]); break;                                            // DEC H  DEC XH  DEC YH
            case 0x26: ld_r_imm(regs[XYState]); break;                                                  // LD H,N  LD XH,N  LD YH,N
            case 0x27: mathOp< iDaa >( regA ); break;                                                   // DAA
            case 0x28: jr( regF.getZ() ); break;                                                        // JR Z,N
            case 0x29: mathOp16< iAdd >( regPairs[XYState], regPairs[XYState] ); break;                 // ADD HL,HL  ADD IX,IX  ADD IY,IY
            case 0x2A: ld_rr_mem( regPairs[XYState] ); break;                                           // LD HL,(NN)  LD IX,(NN)  LD IY,(NN)
            case 0x2B: regPairs[XYState]--; break;                                                      // DEC HL  DEC IX  DEC IY
            case 0x2C: mathOp< iInc >(regs[XYState + 1]); break;                                        // INC L  INC XL  INC YL
            case 0X2D: mathOp< iDec >(regs[XYState + 1]); break;                                        // DEC L  DEC XL  DEC YL
            case 0x2E: ld_r_imm(regs[XYState + 1]); break;                                              // LD L,N  LD XL,N  LD YL,N
            case 0x2F: mathOp< iCpl >( regA ); break;                                                   // CPL
            case 0x30: jr( !regF.getC() ); break;                                                       // JR NC,N
            case 0x31: ld_rr_imm( regSP ); break;                                                       // LD SP,NN
            case 0x32: ld_mem_r(regA); break;                                                           // LD (NN),A  
            case 0x33: regSP++; break;                                                                  // INC SP
            case 0x34: inst_indirXY< iInc >(); break;                                                   // INC (HL)  INC (IX)  INC (IY)
            case 0x35: inst_indirXY< iDec >(); break;                                                   // DEC (HL)  DEC (IX)  DEC (IY)
            case 0x36: ld_indirXY_imm(); break;                                                         // LD (HL),N  LD (IX+d),N  LD (IY+d),N
            case 0x37: regF.setC(); flagsScfCcf(); break;                                               // SCF
            case 0x38: jr( regF.getC() ); break;                                                        // JR C,N
            case 0x39: mathOp16< iAdd >( regPairs[XYState], regSP ); break;                             // ADD HL,SP  ADD IX,SP  ADD IY,SP
            case 0x3A: ld_r_mem(regA); break;                                                           // LD A,(NN)
            case 0x3B: regSP--; break;                                                                  // DEC SP
            case 0x3C: mathOp< iInc >(regA); break;                                                     // INC A
            case 0X3D: mathOp< iDec >(regA); break;                                                     // DEC A
            case 0x3E: ld_r_imm(regA); break;                                                           // LD A,N
            case 0x3F: regF.negC(); flagsScfCcf(); break;                                               // CCF
            case 0x40: break;                                                                           // LD B,B
            case 0x41: regs[rB] = regs[rC]; break;                                                      // LD B,C
            case 0x42: regs[rB] = regs[rD]; break;                                                      // LD B,D
            case 0x43: regs[rB] = regs[rE]; break;                                                      // LD B,E
            case 0x44: regs[rB] = regs[XYState]; break;                                                 // LD B,H  LD B,XH  LD B,YH
            case 0x45: regs[rB] = regs[XYState + 1]; break;                                             // LD B,L  LD B,XL  LD B,YL
            case 0x46: ld_r_indirXY(regs[rB]); break;                                                   // LD B,(HL)  LD B,(IX+d)  LD B,(IY+d)
            case 0x47: regs[rB] = regA; break;                                                          // LD B,A
            case 0x48: regs[rC] = regs[rB]; break;                                                      // LD C,B
            case 0x49: break;                                                                           // LD C,C
            case 0x4A: regs[rC] = regs[rD]; break;                                                      // LD C,D
            case 0x4B: regs[rC] = regs[rE]; break;                                                      // LD C,E
            case 0x4C: regs[rC] = regs[XYState]; break;                                                 // LD C,H  LD C,XH  LD C,YH
            case 0x4D: regs[rC] = regs[XYState + 1]; break;                                             // LD C,L  LD C,XL  LD C,YL
            case 0x4E: ld_r_indirXY(regs[rC]); break;                                                   // LD C,(HL)  LD C,(IX+d)  LD C,(IY+d)
            case 0x4F: regs[rC] = regA; break;                                                          // LD C,A
            case 0x50: regs[rD] = regs[rB]; break;                                                      // LD D,B
            case 0x51: regs[rD] = regs[rC]; break;                                                      // LD D,C
            case 0x52: break;                                                                           // LD D,D
            case 0x53: regs[rD] = regs[rE]; break;                                                      // LD D,E
            case 0x54: regs[rD] = regs[XYState]; break;                                                 // LD D,H  LD D,XH  LD D,YH
            case 0x55: regs[rD] = regs[XYState + 1]; break;                                             // LD D,L  LD D,XL  LD D,YL
            case 0x56: ld_r_indirXY(regs[rD]); break;                                                   // LD D,(HL)  LD D,(IX+d)  LD D,(IY+d)
            case 0x57: regs[rD] = regA; break;                                                          // LD D,A
            case 0x58: regs[rE] = regs[rB]; break;                                                      // LD E,B
            case 0x59: regs[rE] = regs[rC]; break;                                                      // LD E,C
            case 0x5A: regs[rE] = regs[rD]; break;                                                      // LD E,D
            case 0x5B: break;                                                                           // LD E,E
            case 0x5C: regs[rE] = regs[XYState]; break;                                                 // LD E,H  LD E,XH  LD E,YH
            case 0x5D: regs[rE] = regs[XYState + 1]; break;                                             // LD E,L  LD E,XL  LD E,YL
            case 0x5E: ld_r_indirXY(regs[rE]); break;                                                   // LD E,(HL)  LD E,(IX+d)  LD E,(IY+d)
            case 0x5F: regs[rE] = regA; break;                                                          // LD E,A
            case 0x60: regs[XYState] = regs[rB]; break;                                                 // LD H,B  LD XH,B  LD YH,B
            case 0x61: regs[XYState] = regs[rC]; break;                                                 // LD H,C  LD XH,C  LD YH,C
            case 0x62: regs[XYState] = regs[rD]; break;                                                 // LD H,D  LD XH,D  LD YH,D
            case 0x63: regs[XYState] = regs[rE]; break;                                                 // LD H,E  LD XH,E  LD YH,E
            case 0x64: break;                                                                           // LD H,H  LD XH,XH  LD XH,YH
            case 0x65: regs[XYState] = regs[XYState + 1]; break;                                        // LD H,L  LD XH,XL  LD XH,YL
            case 0x66: ld_r_indirXY(regs[rH]); break;                                                   // LD H,(HL)  LD H,(IX+d)  LD H,(IY+d)
            case 0x67: regs[XYState] = regA; break;                                                     // LD H,A  LD XH,A  LD YH,A
            case 0x68: regs[XYState + 1] = regs[rB]; break;                                             // LD L,B  LD XL,B  LD YL,B
            case 0x69: regs[XYState + 1] = regs[rC]; break;                                             // LD L,C  LD XL,B  LD YL,B
            case 0x6A: regs[XYState + 1] = regs[rD]; break;                                             // LD L,D  LD XL,B  LD YL,B
            case 0x6B: regs[XYState + 1] = regs[rE]; break;                                             // LD L,E  LD XL,B  LD YL,B
            case 0x6C: regs[XYState + 1] = regs[XYState]; break;                                        // LD L,H  LD XL,XH  LD YL,YH
            case 0x6D: break;                                                                           // LD L,L  LD XL,XL  LD YL,YL
            case 0x6E: ld_r_indirXY(regs[rL]); break;                                                   // LD L,(HL)  LD L,(IX+d)  LD L,(IY+d)
            case 0x6F: regs[XYState + 1] = regA; break;                                                 // LD L,A  LD XL,A  LD YL,A
            case 0x70: ld_indirXY_r(regs[rB]); break;                                                   // LD (HL),B  LD (IX+d),B  LD (IY+d),B
            case 0x71: ld_indirXY_r(regs[rC]); break;                                                   // LD (HL),C  LD (IX+d),C  LD (IY+d),C
            case 0x72: ld_indirXY_r(regs[rD]); break;                                                   // LD (HL),D  LD (IX+d),D  LD (IY+d),D
            case 0x73: ld_indirXY_r(regs[rE]); break;                                                   // LD (HL),E  LD (IX+d),E  LD (IY+d),E
            case 0x74: ld_indirXY_r(regs[rH]); break;                                                   // LD (HL),H  LD (IX+d),H  LD (IY+d),H
            case 0x75: ld_indirXY_r(regs[rL]); break;                                                   // LD (HL),L  LD (IX+d),L  LD (IY+d),L
            case 0x76: mc_StateMachine = sHalt; break;                                                  // HALT
            case 0x77: ld_indirXY_r(regA); break;                                                       // LD (HL),A  LD (IX+d),A  LD (IY+d),A
            case 0x78: regA = regs[rB]; break;                                                          // LD A,B
            case 0x79: regA = regs[rC]; break;                                                          // LD A,C
            case 0x7A: regA = regs[rD]; break;                                                          // LD A,D
            case 0x7B: regA = regs[rE]; break;                                                          // LD A,E
            case 0x7C: regA = regs[XYState]; break;                                                     // LD A,H  LD A,XH  LD A,YH
            case 0x7D: regA = regs[XYState + 1]; break;                                                 // LD A,L  LD A,XL  LD A,YL
            case 0x7E: ld_r_indirXY(regA); break;                                                       // LD A,(HL)  LD A,(IX+d)  LD A,(IY+d)
            case 0x7F: break;                                                                           // LD A,A
            case 0x80: mathOp< iAdd >( regA, regs[rB] ); break;                                         // ADD A,B
            case 0x81: mathOp< iAdd >( regA, regs[rC] ); break;                                         // ADD A,C
            case 0x82: mathOp< iAdd >( regA, regs[rD] ); break;                                         // ADD A,D
            case 0x83: mathOp< iAdd >( regA, regs[rE] ); break;                                         // ADD A,E
            case 0x84: mathOp< iAdd >( regA, regs[XYState] ); break;                                    // ADD A,H  ADD A,XH  ADD A,YH
            case 0x85: mathOp< iAdd >( regA, regs[XYState + 1] ); break;                                // ADD A,L  ADD A,XL  ADD A,YL
            case 0x86: inst_indirXY< iAdd >(); break;                                                   // ADD A,(HL)  ADD A,(IX+d)  ADD A,(IY+d)
            case 0x87: mathOp< iAdd >( regA, regA ); break;                                             // ADD A,A
            case 0x88: mathOp< iAdc >( regA, regs[rB] ); break;                                         // ADC A,B
            case 0x89: mathOp< iAdc >( regA, regs[rC] ); break;                                         // ADC A,C
            case 0x8A: mathOp< iAdc >( regA, regs[rD] ); break;                                         // ADC A,D
            case 0x8B: mathOp< iAdc >( regA, regs[rE] ); break;                                         // ADC A,E
            case 0x8C: mathOp< iAdc >( regA, regs[XYState] ); break;                                    // ADC A,H  ADC A,XH  ADC A,YH
            case 0x8D: mathOp< iAdc >( regA, regs[XYState + 1] ); break;                                // ADC A,L  ADC A,XL  ADC A,YL
            case 0x8E: inst_indirXY<iAdc>(); break;                                                     // ADC A,(HL)  ADC A,(IX+d)  ADC A,(IY+d)
            case 0x8F: mathOp< iAdc >( regA, regA ); break;                                             // ADC A,A
            case 0x90: mathOp< iSub >( regA, regs[rB] ); break;                                         // SUB B
            case 0x91: mathOp< iSub >( regA, regs[rC] ); break;                                         // SUB C
            case 0x92: mathOp< iSub >( regA, regs[rD] ); break;                                         // SUB D
            case 0x93: mathOp< iSub >( regA, regs[rE] ); break;                                         // SUB E
            case 0x94: mathOp< iSub >( regA, regs[XYState] ); break;                                    // SUB H  SUB XH  SUB YH
            case 0x95: mathOp< iSub >( regA, regs[XYState + 1] ); break;                                // SUB L  SUB XL  SUB YL
            case 0x96: inst_indirXY<iSub>(); break;                                                     // SUB (HL)  SUB (IX+d)  SUB (IY+d)
            case 0x97: mathOp< iSub >( regA, regA ); break;                                             // SUB A
            case 0x98: mathOp< iSbc >( regA, regs[rB] ); break;                                         // SBC A,B
            case 0x99: mathOp< iSbc >( regA, regs[rC] ); break;                                         // SBC A,C
            case 0x9A: mathOp< iSbc >( regA, regs[rD] ); break;                                         // SBC A,D
            case 0x9B: mathOp< iSbc >( regA, regs[rE] ); break;                                         // SBC A,E
            case 0x9C: mathOp< iSbc >( regA, regs[XYState] ); break;                                    // SBC A,H  SBC A,XH  SBC A,YH
            case 0x9D: mathOp< iSbc >( regA, regs[XYState + 1] ); break;                                // SBC A,L  SBC A,XL  SBC A,YL
            case 0x9E: inst_indirXY<iSbc>(); break;                                                     // SBC A,(HL)  SBC A,(IX+d)  SBC A,(IY+d)
            case 0x9F: mathOp< iSbc >( regA, regA ); break;                                             // SBC A,A
            case 0xA0: mathOp< iAnd >( regA,regs[rB]); break;                                           // AND B
            case 0xA1: mathOp< iAnd >( regA,regs[rC]); break;                                           // AND C
            case 0xA2: mathOp< iAnd >( regA,regs[rD]); break;                                           // AND D
            case 0xA3: mathOp< iAnd >( regA,regs[rE]); break;                                           // AND E
            case 0xA4: mathOp< iAnd >( regA,regs[XYState]); break;                                      // AND H  AND XH  AND YL
            case 0xA5: mathOp< iAnd >( regA,regs[XYState + 1]); break;                                  // AND L  AND XL  AND YL
            case 0xA6: inst_indirXY<iAnd>(); break;                                                     // AND (HL)  AND (IX+d)  AND (IY+d)
            case 0xA7: mathOp< iAnd >( regA,regA); break;                                               // AND A
            case 0xA8: mathOp< iXor >( regA,regs[rB]); break;                                           // XOR B
            case 0xA9: mathOp< iXor >( regA,regs[rC]); break;                                           // XOR C
            case 0xAA: mathOp< iXor >( regA,regs[rD]); break;                                           // XOR D
            case 0xAB: mathOp< iXor >( regA,regs[rE]); break;                                           // XOR E
            case 0xAC: mathOp< iXor >( regA,regs[XYState]); break;                                      // XOR H  XOR XH  XOR YH
            case 0xAD: mathOp< iXor >( regA,regs[XYState + 1]); break;                                  // XOR L  XOR XL  XOR YL
            case 0xAE: inst_indirXY<iXor>(); break;                                                     // XOR (HL)  XOR (IX+d)  XOR (IY+d)
            case 0xAF: mathOp< iXor >( regA,regA); break;                                               // XOR A
            case 0xB0: mathOp< iOr >( regA,regs[rB]); break;                                            // OR B
            case 0xB1: mathOp< iOr >( regA,regs[rC]); break;                                            // OR C
            case 0xB2: mathOp< iOr >( regA,regs[rD]); break;                                            // OR D
            case 0xB3: mathOp< iOr >( regA,regs[rE]); break;                                            // OR E
            case 0xB4: mathOp< iOr >( regA,regs[XYState]); break;                                       // OR H  OR XH  OR YL
            case 0xB5: mathOp< iOr >( regA,regs[XYState + 1]); break;                                   // OR L  OR XL  OR YL
            case 0xB6: inst_indirXY<iOr>(); break;                                                      // OR (HL)  OR (IX+d)  OR (IY+d)
            case 0xB7: mathOp< iOr >( regA,regA); break;                                                // OR A
            case 0xB8: mathOp< iCp >( regA,regs[rB]); break;                                            // CP B
            case 0xB9: mathOp< iCp >( regA,regs[rC]); break;                                            // CP C
            case 0xBA: mathOp< iCp >( regA,regs[rD]); break;                                            // CP D
            case 0xBB: mathOp< iCp >( regA,regs[rE]); break;                                            // CP E
            case 0xBC: mathOp< iCp >( regA,regs[XYState]); break;                                       // CP H  CP XH  CP YH
            case 0xBD: mathOp< iCp >( regA,regs[XYState + 1]); break;                                   // CP L  CP XL  CP YL
            case 0xBE: inst_indirXY<iCp>(); break;                                                      // CP (HL)  CP (IX+d)  CP (IY+d)
            case 0xBF: mathOp< iCp >( regA,regA); break;                                                // CP A
            case 0xC0: ret( !regF.getZ() ); break;                                                      // RET NZ
            case 0xC1: pop_rr( regPairs[rBC] ); break;                                                  // POP BC
            case 0xC2: jp( !regF.getZ() ); break;                                                       // JP NZ,NN
            case 0xC3: jp(true); break;                                                                 // JP NN
            case 0xC4: call( !regF.getZ() ); break;                                                     // CALL NZ,NN
            case 0xC5: push_rr( regPairs[rBC] ); break;                                                 // PUSH BC
            case 0xC6: inst_imm<iAdd>(); break;                                                         // ADD A,N
            case 0xC7: rst( 0x00 ); break;                                                              // RST 00H
            case 0xC8: ret( regF.getZ() ); break;                                                       // RET Z
            case 0xC9: ret(true); break;                                                                // RET
            case 0xCA: jp( regF.getZ() ); break;                                                        // JP Z,NN
            case 0xCB: prefix_cb(); break;                                                              // PREFIX CB
            case 0xCC: call( regF.getZ() ); break;                                                      // CALL Z,NN
            case 0xCD: call(true); break;                                                               // CALL NN
            case 0xCE: inst_imm<iAdc>(); break;                                                         // ADC A,N
            case 0xCF: rst( 0x08 ); break;                                                              // RST 08H
            case 0xD0: ret( !regF.getC() ); break;                                                      // RET NC
            case 0xD1: pop_rr( regPairs[rDE] ); break;                                                  // POP DE
            case 0xD2: jp( !regF.getC() ); break;                                                       // JP NC,NN
            case 0xD3: out_imm_r(regA); break;                                                          // OUT (N),A
            case 0xD4: call( !regF.getC() ); break;                                                     // CALL NC,NN
            case 0xD5: push_rr( regPairs[rDE] ); break;                                                 // PUSH DE
            case 0xD6: inst_imm<iSub>(); break;                                                         // SUB N
            case 0xD7: rst( 0x10 ); break;                                                              // RST 10H
            case 0xD8: ret( regF.getC() ); break;                                                       // RET C
            case 0xD9: ex_rr_rr( regPairs[rBC], regsAlt[rB], regsAlt[rC] );                             // EXX
                       ex_rr_rr( regPairs[rDE], regsAlt[rD], regsAlt[rE] );
                       ex_rr_rr( regPairs[rHL], regsAlt[rH], regsAlt[rL] );
                       break;
            case 0xDA: jp( regF.getC() ); break;                                                        // JP C,NN
            case 0xDB: in_r_imm(regA); break;                                                           // IN A,(N)
            case 0xDC: call( regF.getC() ); break;                                                      // CALL C,NN
            case 0xDD: mc_prefix = prefixIX; break;                                                     // PREFIX IX
            case 0xDE: inst_imm<iSbc>(); break;                                                         // SBC A,N
            case 0xDF: rst( 0x18 ); break;                                                              // RST 18H
            case 0xE0: ret( !regF.getP() ); break;                                                      // RET PO
            case 0xE1: pop_rr( regPairs[XYState] ); break;                                              // POP HL  POP IX  POP IY
            case 0xE2: jp( !regF.getP() ); break;                                                       // JP PO,NN
            case 0xE3: ex_SP_rr( regPairs[XYState] ); break;                                            // EX (SP),HL  EX (SP),IX  EX (SP),IY
            case 0xE4: call( !regF.getP() ); break;                                                     // CALL PO,NN
            case 0xE5: push_rr( regPairs[XYState] ); break;                                             // PUSH HL  PUSH IX  PUSH IY
            case 0xE6: inst_imm<iAnd>(); break;                                                         // AND N
            case 0xE7: rst( 0x20 ); break;                                                              // RST 20H
            case 0xE8: ret( regF.getP() ); break;                                                       // RET PE
            case 0xE9: m_PC = regPairs[XYState];                                                        // JP (HL)  JP (IX)  JP (IY)
                       sAO = m_PC;
                       break;
            case 0xEA: jp( regF.getP() ); break;                                                        // JP PE,NN
            case 0xEB: ex_rr_rr( regPairs[rDE], regs[rH], regs[rL] ); break;                            // EX DE,HL
            case 0xEC: call( regF.getP() ); break;                                                      // CALL PE,NN
            case 0xED: mc_prefix = prefixED; break;                                                     // PREFIX ED
            case 0xEE: inst_imm<iXor>(); break;                                                         // XOR N
            case 0xEF: rst( 0x28 ); break;                                                              // RST 28H
            case 0xF0: ret( !regF.getS() ); break;                                                      // RET P
            case 0xF1: pop_rr( regPairs[rAF] ); break;                                                  // POP AF
            case 0xF2: jp( !regF.getS() ); break;                                                       // JP P,NN
            case 0xF3: IFF1 = false; IFF2 = false; break;                                               // DI
            case 0xF4: call( !regF.getS() ); break;                                                     // CALL P,NN
            case 0xF5: push_rr( regPairs[rAF] ); break;                                                 // PUSH AF
            case 0xF6: inst_imm<iOr>(); break;                                                          // OR N
            case 0xF7: rst( 0x30 ); break;                                                              // RST 30H
            case 0xF8: ret( regF.getS() ); break;                                                       // RET M
            case 0xF9: regSP = regPairs[XYState]; break;                                                // LD SP,HL  LD SP,IX  LP SP,IY
            case 0xFA: jp( regF.getS() ); break;                                                        // JP M,NN
            case 0xFB: IFF1 = true; IFF2 = true; sInt = false; break;                                   // EI
            case 0xFC: call( regF.getS() ); break;                                                      // CALL M,NN
            case 0xFD: mc_prefix = prefixIY; break;                                                     // PREFIX IY
            case 0xFE: inst_imm<iCp>(); break;                                                          // CP N
            case 0xFF: rst( 0x38 ); break;                                                              // RST 38H
        }

    // Execute instructions with prefix CB
    if (m_iSet == prefixCB)
        switch( m_iReg & 0x07 ) {
            // RLC B  RRC B  RL B  RR B  SLA B  SRA B  SLL B  SRL B  BIT n,B  RES n,B  SET n,B
            // LD B,RLC(IX+d)  LD B,RRC(IX+d)  LD B,RL(IX+d)  LD B,RR(IX+d)  LD B,SLA(IX+d)  LD B,SRA(IX+d)  LD B,SRL(IX+d)  LD B,SRL(IX+d)  LD B,RES n,(IX+d)  LD B,SET n,(IX+d)
            // LD B,RLC(IY+d)  LD B,RRC(IY+d)  LD B,RL(IY+d)  LD B,RR(IY+d)  LD B,SLA(IY+d)  LD B,SRA(IY+d)  LD B,SRL(IY+d)  LD B,SRL(IY+d)  LD B,RES n,(IY+d)  LD B,SET n,(IY+d)
            case 0: instCB( m_iReg, regs[rB] ); break;
            // RLC C  RRC C  RL C  RR C  SLA C  SRA C  SLL C  SRL C  BIT n,C  RES n,C  SET n,C
            // LD C,RLC(IX+d)  LD C,RRC(IX+d)  LD C,RL(IX+d)  LD C,RR(IX+d)  LD C,SLA(IX+d)  LD C,SRA(IX+d)  LD C,SRL(IX+d)  LD C,SRL(IX+d)  LD C,RES n,(IX+d)  LD C,SET n,(IX+d)
            // LD C,RLC(IY+d)  LD C,RRC(IY+d)  LD C,RL(IY+d)  LD C,RR(IY+d)  LD C,SLA(IY+d)  LD C,SRA(IY+d)  LD C,SRL(IY+d)  LD C,SRL(IY+d)  LD C,RES n,(IY+d)  LD C,SET n,(IY+d)
            case 1: instCB( m_iReg, regs[rC] ); break;
            // RLC D  RRC D  RL D  RR D  SLA D  SRA D  SLL D  SRL D  BIT n,D  RES n,D  SET n,D
            // LD D,RLC(IX+d)  LD D,RRC(IX+d)  LD D,RL(IX+d)  LD D,RR(IX+d)  LD D,SLA(IX+d)  LD D,SRA(IX+d)  LD D,SRL(IX+d)  LD D,SRL(IX+d)  LD D,RES n,(IX+d)  LD D,SET n,(IX+d)
            // LD D,RLC(IY+d)  LD D,RRC(IY+d)  LD D,RL(IY+d)  LD D,RR(IY+d)  LD D,SLA(IY+d)  LD D,SRA(IY+d)  LD D,SRL(IY+d)  LD D,SRL(IY+d)  LD D,RES n,(IY+d)  LD D,SET n,(IY+d)
            case 2: instCB( m_iReg, regs[rD] ); break;
            // RLC E  RRC E  RL E  RR E  SLA E  SRA E  SLL E  SRL E  BIT n,E  RES n,E  SET n,E
            // LD E,RLC(IX+d)  LD E,RRC(IX+d)  LD E,RL(IX+d)  LD E,RR(IX+d)  LD E,SLA(IX+d)  LD E,SRA(IX+d)  LD E,SRL(IX+d)  LD E,SRL(IX+d)  LD E,RES n,(IX+d)  LD E,SET n,(IX+d)
            // LD E,RLC(IY+d)  LD E,RRC(IY+d)  LD E,RL(IY+d)  LD E,RR(IY+d)  LD E,SLA(IY+d)  LD E,SRA(IY+d)  LD E,SRL(IY+d)  LD E,SRL(IY+d)  LD E,RES n,(IY+d)  LD E,SET n,(IY+d)
            case 3: instCB( m_iReg, regs[rE] ); break;
            // RLC H  RRC H  RL H  RR H  SLA H  SRA H  SLL H  SRL H  BIT n,H  RES n,H  SET n,H
            // LD H,RLC(IX+d)  LD H,RRC(IX+d)  LD H,RL(IX+d)  LD H,RR(IX+d)  LD H,SLA(IX+d)  LD H,SRA(IX+d)  LD H,SRL(IX+d)  LD H,SRL(IX+d)  LD H,RES n,(IX+d)  LD H,SET n,(IX+d)
            // LD H,RLC(IY+d)  LD H,RRC(IY+d)  LD H,RL(IY+d)  LD H,RR(IY+d)  LD H,SLA(IY+d)  LD H,SRA(IY+d)  LD H,SRL(IY+d)  LD H,SRL(IY+d)  LD H,RES n,(IY+d)  LD H,SET n,(IY+d)
            case 4: instCB( m_iReg, regs[rH] ); break;
            // RLC L  RRC L  RL L  RR L  SLA L  SRA L  SLL L  SRL L  BIT n,L  RES n,L  SET n,L
            // LD L,RLC(IX+d)  LD L,RRC(IX+d)  LD L,RL(IX+d)  LD L,RR(IX+d)  LD L,SLA(IX+d)  LD L,SRA(IX+d)  LD L,SRL(IX+d)  LD L,SRL(IX+d)  LD L,RES n,(IX+d)  LD L,SET n,(IX+d)
            // LD L,RLC(IY+d)  LD L,RRC(IY+d)  LD L,RL(IY+d)  LD L,RR(IY+d)  LD L,SLA(IY+d)  LD L,SRA(IY+d)  LD L,SRL(IY+d)  LD L,SRL(IY+d)  LD L,RES n,(IY+d)  LD L,SET n,(IY+d)
            case 5: instCB( m_iReg, regs[rL] ); break;
            // RLC (HL)  RRC (HL)  RL (HL)  RR (HL)  SLA (HL)  SRA (HL)  SLL (HL)  SRL (HL)  BIT n,(HL)  RES n,(HL)  SET n,(HL)
            // RLC (IX+d)  RRC (IX+d)  RL (IX+d)  RR (IX+d)  SLA (IX+d)  SRA (IX+d)  SLL (IX+d)  SRL (IX+d)  BIT n,(IX+d)  RES n,(IX+d)  SET n,(IX+d)
            // RLC (IY+d)  RRC (IY+d)  RL (IY+d)  RR (IY+d)  SLA (IY+d)  SRA (IY+d)  SLL (IY+d)  SRL (IY+d)  BIT n,(IY+d)  RES n,(IY+d)  SET n,(IY+d)
            case 6: { uint8_t none; instCB( m_iReg, none ); } break;
            // RLC A  RRC A  RL A  RR A  SLA A  SRA A  SLL A  SRL A  BIT n,A  RES n,A  SET n,A
            // LD A,RLC(IX+d)  LD A,RRC(IX+d)  LD A,RL(IX+d)  LD A,RR(IX+d)  LD A,SLA(IX+d)  LD A,SRA(IX+d)  LD A,SRL(IX+d)  LD A,SRL(IX+d)  LD A,RES n,(IX+d)  LD A,SET n,(IX+d)
            // LD A,RLC(IY+d)  LD A,RRC(IY+d)  LD A,RL(IY+d)  LD A,RR(IY+d)  LD A,SLA(IY+d)  LD A,SRA(IY+d)  LD A,SRL(IY+d)  LD A,SRL(IY+d)  LD A,RES n,(IY+d)  LD A,SET n,(IY+d)
            case 7: instCB( m_iReg, regA ); break;                                          
        }
        
    // Execute instructions with prefix ED
    if (m_iSet == prefixED)
        switch(m_iReg) {
            case 0x40: in_r_rr( regs[rB], regPairs[rBC] ); break;                                       // IN B,(C)
            case 0x41: out_rr_r( regPairs[rBC], regs[rB] ); break;                                      // OUT (C),B
            case 0x42: mathOp16< iSbc >( regPairs[rHL], regPairs[rBC] ); break;                         // SBC HL,BC
            case 0x43: ld_mem_rr( regPairs[rBC] ); break;                                               // LD (NN),BC
            case 0x44: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x45: retn(); break;                                                                   // RETN
            case 0x46: intMode = 0; break;                                                              // IM 0
            case 0x47: regI = regA; break;                                                              // LD I,A
            case 0x48: in_r_rr( regs[rC], regPairs[rBC] ); break;                                       // IN C,(C)
            case 0x49: out_rr_r( regPairs[rBC], regs[rC] ); break;                                      // OUT (C),C
            case 0x4A: mathOp16< iAdc >( regPairs[rHL], regPairs[rBC] ); break;                         // ADC HL,BC
            case 0x4B: ld_rr_mem( regPairs[rBC] ); break;                                               // LD BC,(NN)
            case 0x4C: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x4D: retn(); break;                                                                   // RETI
            case 0x4E: intMode = 0; break;                                                              // IM 0/1
            case 0x4F: regR = regA; break;                                                              // LD R,A
            case 0x50: in_r_rr( regs[rD], regPairs[rBC] ); break;                                       // IN D,(C)
            case 0x51: out_rr_r( regPairs[rBC], regs[rD]); break;                                       // OUT (C),D
            case 0x52: mathOp16< iSbc >( regPairs[rHL], regPairs[rDE] ); break;                         // SBC HL,DE
            case 0x53: ld_mem_rr( regPairs[rDE] ); break;                                               // LD (NN),DE
            case 0x54: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x55: retn(); break;                                                                   // RETN
            case 0x56: intMode = 1; break;                                                              // IM 1
            case 0x57: regA = regI; regF.copySYXzeroZ( regA ).resetHN().storeP( IFF2 ); break;          // LD A,I
            case 0x58: in_r_rr( regs[rE], regPairs[rBC] ); break;                                       // IN E,(C)
            case 0x59: out_rr_r( regPairs[rBC], regs[rE] ); break;                                      // OUT (C),E
            case 0x5A: mathOp16< iAdc >( regPairs[rHL], regPairs[rDE] ); break;                         // ADC HL,DE
            case 0x5B: ld_rr_mem( regPairs[rDE] ); break;                                               // LD DE,(NN)
            case 0x5C: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x5D: retn(); break;                                                                   // RETN
            case 0x5E: intMode = 2; break;                                                              // IM 2
            case 0x5F: regA = regR; regF.copySYXzeroZ( regA ).resetHN().storeP( IFF2 ); break;          // LD A,R
            case 0x60: in_r_rr( regs[rH], regPairs[rBC] ); break;                                       // IN H,(C)
            case 0x61: out_rr_r( regPairs[rBC], regs[rH] ); break;                                      // OUT (C),H
            case 0x62: mathOp16< iSbc >( regPairs[rHL], regPairs[rHL] ); break;                         // SBC HL,HL
            case 0x63: ld_mem_rr( regPairs[rHL] ); break;                                               // LD (NN),HL
            case 0x64: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x65: retn(); break;                                                                   // RETN
            case 0x66: intMode = 0; break;                                                              // IM 0
            case 0x67: rlrd< iRight >(); break;                                                         // RRD
            case 0x68: in_r_rr( regs[rL], regPairs[rBC] ); break;                                       // IN L,(C)
            case 0x69: out_rr_r( regPairs[rBC], regs[rL] ); break;                                      // OUT (C),L
            case 0x6A: mathOp16< iAdc >( regPairs[rHL], regPairs[rHL] ); break;                         // ADC HL,HL
            case 0x6B: ld_rr_mem( regPairs[rHL] ); break;                                               // LD HL,(NN)
            case 0x6C: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x6D: retn(); break;                                                                   // RETN
            case 0x6E: intMode = 0; break;                                                              // IM 0/1
            case 0x6F: rlrd< iLeft >(); break;                                                          // RLD
            case 0x70: { uint8_t none; in_r_rr( none, regPairs[rBC] ); } break;                         // IN (C)
            case 0x71: out_rr_r( regPairs[rBC], (m_cmos == false) ? 0x00 : 0xff ); break;               // OUT (C),0  OUT(C),FF
            case 0x72: mathOp16< iSbc >( regPairs[rHL], regSP ); break;                                 // SBC HL,SP
            case 0x73: ld_mem_rr( regSP ); break;                                                       // LD (NN),SP
            case 0x74: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x75: retn(); break;                                                                   // RETN
            case 0x76: intMode = 1; break;                                                              // IM 1
                                                                                                        // 0x77 undocumented
            case 0x78: in_r_rr( regA, regPairs[rBC] ); break;                                           // IN A,(C)
            case 0x79: out_rr_r( regPairs[rBC], regA ); break;                                          // OUT (C),A
            case 0x7A: mathOp16< iAdc >( regPairs[rHL], regSP ); break;                                 // ADC HL,SP
            case 0x7B: ld_rr_mem( regSP ); break;                                                       // LD SP,(NN)
            case 0x7C: mathOp< iNeg >( regA ); break;                                                   // NEG
            case 0x7D: retn(); break;                                                                   // RETN
            case 0x7E: intMode = 2; break;                                                              // IM 2
                                                                                                        // 0x7F undocumented
            case 0xA0: ldxx<bInc>(); break;                                                             // LDI
            case 0xA1: cpxx<bInc>(); break;                                                             // CPI
            case 0xA2: inxx<bInc>(); break;                                                             // INI
            case 0xA3: otxx<bInc>(); break;                                                             // OUTI

            case 0xA8: ldxx<bDec>(); break;                                                             // LDD
            case 0xA9: cpxx<bDec>(); break;                                                             // CPD
            case 0xAA: inxx<bDec>(); break;                                                             // IND
            case 0xAB: otxx<bDec>(); break;                                                             // OUTD

            case 0xB0: ldxx<bIncRep>(); break;                                                          // LDIR
            case 0xB1: cpxx<bIncRep>(); break;                                                          // CPIR
            case 0xB2: inxx<bIncRep>(); break;                                                          // INIR
            case 0xB3: otxx<bIncRep>(); break;                                                          // OTIR

            case 0xB8: ldxx<bDecRep>(); break;                                                          // LDDR
            case 0xB9: cpxx<bDecRep>(); break;                                                          // CPDR
            case 0xBA: inxx<bDecRep>(); break;                                                          // INDR
            case 0xBB: otxx<bDecRep>(); break;                                                          // OTDR
        }
    // If next machine cycle is machine cycle 1, set TStates for next machine cycle - default value for first machine cycle is 4 TStates
    if( sm_MCycle == mc_MCycles ) mc_TStates = 4;
}

void Z80Core::flagsScfCcf() // SCF - set carry flag and CCF - complement carry flag
{
    switch(m_producer) {                                          // different producer of Z80 change flags X and Y differenty
                     // Producer Zilog copy flags X and Y from accumulator when the previous instruction changed register F
                     // otherwise flags X and Y are ored with their previous value
        case pZilog: if( regF.isChanged() == true ) { regF.resetX().resetY(); } break;
                     // Producer NEC always copy flags X and Y from accumulator when the previous instruction changed register F
        case pNec:   regF.resetX().resetY(); break;
                     // Producer ST copy flags Y from accumulator when the previous instruction changed register F otherwise flag Y is ored
                     // with their previous value, flag X is alwasy copied from accumulator when the previous instruction changed register F
        case pSt:    regF.resetX(); if( regF.isChanged() == true ) regF.resetY(); break;
    }
    regF.orYX( regA ).copyNegCtoH().resetN();
}

void Z80Core::ld_r_imm( uint8_t &reg ) //s LD r,N - load register r from byte after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break;                       // next bus op. is read byte from address PC, increase program counter PC
        case 2: reg = sDI;                                      // store byte after instruction to register
    }
}

void Z80Core::ld_r_indir(uint8_t &reg, const Z80RegPair &rp ) //s LD A,(BC) and LD A,(DE) - load register from memory address stored in register pair
{
    switch(sm_MCycle) {
        case 1: regWZ = rp;                                     // set memory pointer
                readMem( regWZ );                               // next bus op. is read byte from memory
                break;
        case 2: reg = sDI;                                      // store byte from memory to register
                regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::ld_r_indirXY( uint8_t &reg )//s LD r,(HL), LD r,(IX+d) and LD r,(IY+d) - load resister from memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if( XYState != rHL ) {                          // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    readMem( m_PC );                              // next bus op. is read byte from memory at address PC
                } else
                    readMem( regPairs[rHL] );      // next bus op. is read byte from memory
                break;
                // Read byte from memory and store it to register
        case 2: reg = sDI;                                      // store byte from memory to register
                break;
                // Address offset fetch
        case 6: regWZ = regPairs[XYState] + static_cast<signed char>(sDI);   // calculation of memory pointer
                noBusOp( m_PC, 5 );                                  // next bus op. is no operation
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: readMem( regWZ );                               // next bus op. is read byte from memory
                m_PC++;                                        // increase program counter PC
    }
}

void Z80Core::ld_r_mem( uint8_t &reg )// LD A,(NN) - load register from memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break; // next bus op. is read byte from address PC, increase program counter PC
                // Read low byte of address and store it to memory pointer
        case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
                readMem( m_PC++ ); break; // next bus op. is read byte from address PC, increase program counter PC
                // Read high byte of address and store it to memory pointer
        case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
                readMem( regWZ );                            // next bus op. is read byte from memory
                break;
                // Read byte from memory and store it to register
        case 4: reg = sDI;                                      // store byte from memory to register
                regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::ld_indir_r( const Z80RegPair &rp, const uint8_t &reg )//s LD (BC),A and LD (DE),A - store register to memory address stored in register pair
{
    switch(sm_MCycle) {
        case 1: regWZ = rp;                     // set memory pointer
                writeMem( regWZ, reg ); break;
                // Write byte from register to memory
        case 2: regWZ++;                                        // increase memory pointer
                regWZ.setHighByte( regA );        // accumulator is stored to high byte of memory pointer
    }
}

void Z80Core::ld_indirXY_r( const uint8_t &reg )//s LD (HL),r, LD (IX+d),r and LD (IY+d),r - store resister r to memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    readMem( m_PC );                              // next bus op. is read byte from memory at address PC
                }
                else writeMem( regPairs[rHL], reg );
                break;
        case 2: break;// Write byte from register to memory
                // Address offset fetch
        case 6: regWZ = regPairs[XYState] + static_cast<signed char>(sDI);   // calculation of memory pointer
                noBusOp( m_PC, 5 );                                  // next bus op. is no operation
                break;
                // Increase program counter and inicialization writting to memory; following machine cycle is machine cycle 2
        case 7: writeMem( regWZ, reg );                         // next bus op. is write byte to memory
                m_PC++;                                        // increase program counter PC
    }
}

void Z80Core::ld_mem_r( const uint8_t &reg )// LD (NN),A - strore register to memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
        case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
                readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
        case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
                writeMem( regWZ, reg );                         // next bus op. is write byte to memory
                regWZ++;  break;                                // increase memory pointer
                // Write byte from register to memory
        case 4: regWZ.setHighByte( regA );        // accumulator is stored to high byte of memory pointer
                break;
    }
}

void Z80Core::ld_indirXY_imm() //s LD (HL),N, LD (IX+d),N and LD (IY+d),N - store byte after instruction to memory address stored in register pair
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - next machine cycle is machine cycle 6
                }
                readMem( m_PC++ );                                // next bus op. is read byte from memory at address PC, increase program counter PC
                break;
                // Read byte after instruction
        case 2: if (XYState != rHL) writeMem( regWZ, sDI );                // test if it is used register IX or IY
                else writeMem( regPairs[rHL], sDI );          // set address for address bus
                break;
        case 3: break;// Write byte after instruction to memory
                // Address offset fetch
        case 6: regWZ = regPairs[XYState] + static_cast<signed char>(sDI);   // calculation of memory pointer
                mc_StateMachine = sXYFetchFinish;               // set address offset fetch finish flag - next machine cycle is machine cycle 2
                readMem( m_PC++ );
                mc_TStates = 5;                                 // set number of TStates for machine cycle 5
                break;                         // next bus op. is read byte from address PC, increase program counter PC
    }
}

template< typename T >
void Z80Core::ld_rr_imm( T &rp )//s LD rr,NN - load register pair rr from two bytes after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
                // Read byte after instruction and store it to low byte of register pair
        case 2: rp.setLowByte( sDI );                                     // store byte after instruction to low byte of register pair
                readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
                // Read byte after instruction and store it to high byte of register pair
        case 3: rp.setHighByte( sDI );                                     // store byte after instruction to high byte of register pair
                break;
    }
}

template< typename T >
void Z80Core::ld_rr_mem( T &rp ) // LD rr,(NN) - load register pair rr from memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break;            // next bus op. is read byte from memory at address PC
                // Read low byte of address and store it to memory pointer
        case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
                readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
                // Read high byte of address and store it to memory pointer
        case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
                readMem( regWZ );                               // next bus op. is read byte from memory
                regWZ++;                                        // increase memory pointer
                break;
                // Read low byte from memory and store it to low byte of register pair
        case 4: rp.setLowByte( sDI );                                     // store byte from memory to low byte of register pair
                readMem( regWZ );                               // next bus op. is read byte from memory
                break;
                // Read high byte from memory and store it to high byte of register pair
        case 5: rp.setHighByte( sDI );                                     // store byte from memory to high byte of register pair
                break;
    }
}

template< typename T >
void Z80Core::ld_mem_rr( T &rp ) // LD (NN),rr - store register pair rr to memory addres from bytes after instruction
{
    switch(sm_MCycle) {
        case 1: readMem( m_PC++ ); break;                      // next bus op. is read byte from address PC, increase program counter PC
        case 2: regWZ.setLowByte( sDI );              // Read low byte of address and store it to memory pointer
                readMem( m_PC++ ); break;                      // next bus op. is read byte from address PC, increase program counter PC
        case 3: regWZ.setHighByte( sDI );       // Read high byte of address and store it to memory pointer
                writeMem( regWZ, rp.getLowByte() );                     // high byte is stored to high byte of memory pointer
                regWZ++;                                     // increase memory pointer
                break;
        case 4: writeMem( regWZ, rp.getHighByte() ); break;               // Write low byte from low byte of register pair to memory
        case 5: break;                                        // Write high byte from high byte of register pair to memory
    }
}

void Z80Core::push_rr( Z80RegPair &rp ) // PUSH rr - store register pair rr to stack
{
    switch(sm_MCycle) {
        case 1: pushStack8( rp.getHighByte() ); break;                     // decrease SP // Inicialization
        case 2: pushStack8( rp.getLowByte() ); break;                     // decrease SP // Write low byte from low byte of register pair to stack
        case 3: break;                                         // Write high byte from high byte of register pair to stack
    }
}

void Z80Core::pop_rr( Z80RegPair &rp ) // POP rr - load register pair rr from stack
{
    switch(sm_MCycle) {
        case 1: popStack8();
                break;
                // Read low byte from stack to low byte of register pair
        case 2: rp.setLowByte( sDI );                                     // low byte is stored to low byte of register pair
                popStack8();
                break;
                // Read high byte from stack to low byte of register pair
        case 3: rp.setHighByte( sDI );                                     // high byte is stored to high byte of register pair
                break;
    }
}

void Z80Core::ex_rr_rr( Z80RegPair &rp, uint8_t &regH, uint8_t &regL )//s EX AF,AF', EX DE,HL and EXX - exchange registers
{
    uint8_t tmp;
    tmp = rp.getHighByte();                                                // exchange high byte of register pair
    rp.setHighByte( regH );
    regH = tmp;
    tmp = rp.getLowByte();                                                // exchange low byte of register pair
    rp.setLowByte( regL );
    regL = tmp;
}

void Z80Core::ex_SP_rr( Z80RegPair &rp ) // EX (SP),HL, EX (SP),IX and EX (SP),IY - exchange register pair and last value in stack
{
    switch(sm_MCycle) {
        case 1: readMem( regSP );                               // next bus op. is read low byte from stack
                regSP++;                                        // increase stack pointer
                break;
                // Read low byte from stack and exchange it with low byte of register pair
        case 2: regWZ.setLowByte( sDI );                 // low byte from stack is stored to low byte of WZ
                readMem( regSP );                               // next bus op. is read high byte from stack
                mc_TStates = 4;                                 // set number of TStates for machine cycle 3
                break;
                // Write low byte to stack
        case 3: regWZ.setHighByte( sDI );          // high byte from stack is stored to high byte of WZ
                writeMem( regSP, rp.getHighByte() );                        // next bus op. is write low byte to stack
                regSP--;                                        // decrease stack pointer back
                rp.setHighByte( regWZ.getHighByte() );                                     // value read from data bus
                break;
                //  Read high byte from stack and exchange it with high byte of register pair
        case 4: writeMem( regSP, rp.getLowByte() );                         // next bus op. is write high byte to stack
                rp.setLowByte( regWZ.getLowByte() );                                     // high byte is stored to high byte of register pair
                mc_TStates = 5;                                 // set number of TStates for machine cycle 5
                break;
                // Write high byte to stack
        case 5: break;
    }
}

//s INC (HL), INC (IX+d) and INC (IY+d) - increase byte at memory address stored in register pair
//s DEC (HL), DEC (IX+d) and DEC (IY+d) - decrease byte at memory address stored in register pair
//s ADD A,(HL), ADD A,(IX+d) and ADD A,(IY+d) - addition of byte at memory address stored in register pair to accumulator
//s ADC A,(HL), ADC A,(IX+d) and ADC A,(IY+d) - addition of byte at memory address stored in register pair and flag C to accumulator
//s SUB (HL), SUB (IX+d) and SUB (IY+d) - subtraction of byte at memory address stored in register pair from accumulator
//s SBC A,(HL), SBC A,(IX+d) and SBC A,(IY+d) - subtraction of byte at memory address stored in register pair and flag C from accumulator
//s AND (HL), AND (IX+d) and AND (IY+d) - bit operator and of byte at memory address stored in register pair and accumulator
//s XOR (HL), XOR (IX+d) and XOR (IY+d) - bit operator xor of byte at memory address stored in register pair and accumulator
//s OR (HL), OR (IX+d) and OR (IY+d) - bit operator or of byte at memory address stored in register pair and accumulator
//s CP (HL), CP (IX+d) and CP (IY+d) - comparison of byte at memory address stored in register pair with accumulator
// This function is a tempate with paramter iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr or iCp which selects required matematical operation
template< Z80Core::eMathOp op >
inline void Z80Core::inst_indirXY()
{
    switch(sm_MCycle) {
                // Inicialization - if the instruction has prefix then the address offset fetch cycle is inserted
        case 1: if (XYState != rHL) {                           // test if it is used register IX or IY
                    mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag - following machine cycle is machine cycle 6
                    readMem( m_PC );                              // next bus op. is read byte from memory at address PC
                } else {
                    readMem( regPairs[rHL] );      // next bus op. is read byte from memory
                    if ( op == iInc || op == iDec ) mc_TStates = 4;
                }
                break;
                // Read byte from memory and do selected matematical operation with accumulator
        case 2: if (op == iInc || op == iDec ) {
                    mathOp< op >( sDI );
                    if (XYState != rHL) writeMem( regWZ, sDI );     // test if it is used register IX or IY set address for address bus
                    else writeMem( regPairs[rHL], sDI);// set address for address bus
                } else
                    mathOp< op >( regA, sDI );
                break;
        case 3: break;
                // Address offset fetch
        case 6: regWZ = ( regPairs[XYState] ) + static_cast<signed char>(sDI);   // calculation of memory pointer
                noBusOp( m_PC, 5 );                                  // next bus op. is no operation
                break;
                // Increase program counter and inicialization reading from memory; following machine cycle is machine cycle 2
        case 7: readMem( regWZ );                               // next bus op. is read byte from memory
                m_PC++;                                        // increase program counter PC
                if ( op == iInc || op == iDec ) mc_TStates = 4;
    }
}

// ADD A,N - addition of byte after instruction to accumulator
// ADC A,N - addition of byte after instruction and flag C to accumulator
// SUB N - subtraction of byte after instruction from accumulator
// SBC A,N - subtraction of byte after instruction and flag C from accumulator
// AND N - bit operator and of byte after instruction and accumulator
// XOR N - bit operator xor of byte after instruction and accumulator
// OR N - bit operator or of byte after instruction and accumulator
// CP N - comparison of byte after instruction with accumulator
// This function is a tempate with parameter iAdd, iAdc, iSub, iSbc, iAnd, iXor, iOr or iCp which selects required matematical operation
template<Z80Core::eMathOp op>
inline void Z80Core::inst_imm()
{
    switch(sm_MCycle) {
                // Inicialization
        case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from memory at address PC, increase program counter PC
                // Read byte after instruction and do selected matematical operation with accumulator
        case 2: mathOp< op >( regA, sDI );
                break;
    }
}

template< Z80Core::eMathOp op, bool changeAllFlags>
inline void Z80Core::mathOp( uint8_t &regA, uint8_t reg )
{
    uint8_t carry = regA;
    uint16_t tmp;

    switch(op) {
    case iInc: regA++; carry = carry & ~regA; regF.copySYXzeroZ( regA ).overflowHP( carry ).resetN(); break;
    case iDec: regA--; carry = ~carry & regA; regF.copySYXzeroZ( regA ).overflowHP( carry ).setN(); break;
    case iAdc: regA += regF.getC();
    case iAdd: regA += reg;
               carry = (carry & reg) | (~regA & (reg | carry));
               if ( changeAllFlags ) regF.copySYXzeroZ( regA ).overflowHPC( carry ).resetN();
               else regF.copyYX( regA ).overflowHC( carry ).resetN();
               break;
    case iSbc: regA -= regF.getC();
    case iSub: regA -= reg; carry = (~carry & reg) | (regA & (reg | ~carry)); regF.copySYXzeroZ( regA ).overflowHPC( carry ).setN(); break;
    case iAnd: regA &= reg; regF.copySYXzeroZparityP( regA ).setH().resetN().resetC(); break;
    case iXor: regA ^= reg; regF.copySYXzeroZparityP( regA ).resetHN().resetC(); break;
    case iOr : regA |= reg; regF.copySYXzeroZparityP( regA ).resetHN().resetC(); break;
    case iCp : tmp = regA - reg;
               carry = (~carry & reg) | (tmp & (reg | ~carry));
               if ( changeAllFlags ) regF.copySzeroZ( tmp ).copyYX( reg ).overflowHPC( carry ).setN();
               else regF.copySzeroZ( tmp ).copyYX( reg ).overflowHP( carry ).setN();
               break;
    case iCpl: regA = ~regA; regF.copyYX( regA ).setH().setN(); break;
    case iNeg: regA = ~regA + 1; carry |= regA; regF.copySYXzeroZ( regA ).overflowHPC( carry ).setN(); break;
    case iDaa: tmp = regA;                                        // extend accumulator to 16 bits
               if ( !regF.getN() ) {                                    // if flag N is not set the last operation was addition
                   // Decimal adjust after addition
                   if( (tmp & 0x000f) > 9 || regF.getH() ) {          // if low nibble > 9 or flag H is set then the number is not valid and will be adjusted
                       if ((tmp & 0x000f) > 9) regF.setH();                 // if low nibble > 9 it means that it overflows after addition number 6: set flag H
                       else                   regF.resetH();                 // else reset flag H
                       tmp += 0x0006;                                      // add to low nibble difference between number 0x09 and 0x10 in hexadecimal
                   }
                   if( (tmp & 0xfff0) > 0x0090 || regF.getC() ) {     // if high nibble> 9 or flag C is set then the number is not valid and will be adjusted
                       tmp += 0x0060;                                      // add to high nibble difference between number 0x09 and 0x10 in hexadecimal
                   }                                                       // overflow is stored in bit 8 of extended accumulator
               } else {                                                    // if flag N is set the last operation was subtraction
               // Decimal adjust after subtraction
                   if( (tmp & 0x000f) > 9 || regF.getH() ) {          // if low nibble > 9 or flag H is set then the number is not valid and will be adjusted
                       if ((tmp & 0x000f) > 5) regF.resetH();                // if low nibble > 5 it does not borrow after subtraction number 6: reset flag H
                       tmp -= 0x0006;                                      // subtract difference between number 0x09 and 0x10 in hexadecimal from the low nibble
                       tmp &= 0x00ff;                                      // reset borrow in extended accumulator
                   }
                   if( regA > 0x0099 || regF.getC() ) tmp -= 0x0160; // if number > 0x99 or flag C is set then the number is not valid and will be adjusted
                                                               // subtract difference between number 0x09 and 0x10 in hexadecimal  from the high nibble
                                                               // and set borrow in extended accumulator
               }
               regA = tmp & 0x00ff;                                       // copy low byte of extended accumulator to accumulator
               regF.copySYXzeroZparityP( regA );
               if ((tmp & 0xff00) != 0) regF.setC();                        // logical OR overflow or borrow from extended accumulator and flag C
    }
}
    
// DAA - decimal adjust of accumulator
// NEG - accumulator two's complement
// calculates accumulator = (0 - accumulator)
// (0 - minuend, A - subtrahend, 0-A - result)
//
//  0  A  0-A  borrow
//  0  0   0      0
//  0  0   1      1
//  0  1   0      1
//  0  1   1      1
//
// The mathematical equation for borrow is followed
// borrow = A + (0-A)

// ADD rr,rr - addition of register pair rr and other register pair rr
// The addition is calculated in two steps. At first the low bytes are added and then the high bytes are added.
// ADC rr,rr - addition of register pair rr and flag C and other register pair rr
// The addition is calculated in two steps. At first the low bytes are added and then the high bytes are added.
// SBC rr,rr - subtraction of register pair rr and flag C from other regiser pair rr
// The subtraction is calculated in two steps. At first the low bytes are subtracted and then the high bytes are subtracted.
template< Z80Core::eMathOp op, typename T >
inline void Z80Core::mathOp16( Z80RegPair &rp1, T &rp2 )
{
    switch(sm_MCycle) {
    case 1: noBusOp( m_PC, 4 ); break;                           // next bus op. is no operation
            // Addition of low bytes
    case 2: regWZ = rp1;               // set memory pointer
            mathOp< op, false >( rp1.getLowByte(), rp2.getLowByte() );
            noBusOp( m_PC, 3 );                                  // next bus op. is no operation
            break;
            // Addition of high bytes
    case 3: if ( op != iSbc ) mathOp< iAdc, op != iAdd >( rp1.getHighByte(), rp2.getHighByte() );          // store value of high byte before addition for overflow calculation
            else mathOp< iSbc >( rp1.getHighByte(), rp2.getHighByte() );
            if ( op != iAdd ) regF.andZ( rp1.getLowByte() == 0 );
            regWZ++;                                        // increase memory pointer
    }
}

template< Z80Core::eShiftOp op, bool changeSZP>
inline void Z80Core::shiftOp( uint8_t &reg ) {
    bool carry;

    switch(op) {
    case iRlc: carry = reg & 0x80;  reg = (reg << 1) | (reg >> 7); break;                              // rotation 1 bit left
    case iRrc: carry = reg & 0x01;  reg = (reg >> 1) | (reg << 7); break;                              // rotation 1 bit right
    case iRl : carry = reg & 0x80; reg = (reg << 1) | regF.getC(); break;                   // shift 1 bit left and copy flag C to bit 0
    case iRr : carry = reg & 0x01; reg = (reg >> 1) | ( regF.getC() << 7); break;            // shift 1 bit right and copy flag C to bit 7
    case iSla: carry = reg & 0x80; reg = (reg << 1); break;                                          // shift 1 bit left
    case iSra: carry = reg & 0x01; reg = (reg >> 1) | (reg & 0x80); break;                            // shift 1 bit right and keep bit 7
    case iSll: carry = reg & 0x80; reg = (reg << 1) | 0x01; break;                                   // shift 1 bit left and set bit 0
    case iSrl: carry = reg & 0x01; reg = (reg >> 1); break;                                           // shift 1 bit right
    default  : carry = false; // just to prevent compiler warning
    }
    if (changeSZP) regF.copySYXzeroZparityP( reg ).resetHN().storeC( carry );
    else regF.copyYX( reg ).resetHN().storeC( carry );
}

// RLD - rotation left digit
// Rotate 12 bit number 4 bit left where the low 8 bits of the number is stored in memory 
// at address in register pair HL and the high 4 bits of number is stored in accumulator
// RRD - rotation right digit
// Rotate 12 bit number 4 bit right where the low 8 bits of the number is stored in accumulator
// and the high 4 bits of number is stored in memory at address in register pair HL
template< Z80Core::eRotDig op >
void Z80Core::rlrd()
{
    static uint8_t tmp;
    switch(sm_MCycle) {
    case 1: regWZ = regPairs[rHL];             // set memory pointer
            readMem( regWZ );                               // next bus op. is read byte from memory
            break;
            // Read byte from memory and rotate accumulator
    case 2: tmp = regA;                                     // store accumulator for next machine cycle
            if ( op == iLeft ) regA = (regA & 0xf0) | (sDI >> 4);              // copy high nibble of read byte to accumulator
            if ( op == iRight ) regA = (regA & 0xf0) | (sDI & 0x0f);            // copy low nibble of read byte to accumulator
            regF.copySYXzeroZparityP( regA ).resetHN();
            noBusOp( m_PC, 4 );                                  // next bus op. is no operation
            break;
            // Rotate byte from memory
    case 3: if (op == iLeft) writeMem( regWZ, (sDI << 4) | (tmp & 0x0f) );   // copy low nibble of read byte to high nibble and copy low nibble from stored accumulator
            if (op == iRight) writeMem( regWZ, (sDI >> 4) | (tmp << 4) );     // copy high nibble of read byte to low nibble and copy low nibble from stored accumulator
            break;                                          // from previous machine cycle to low nibble
    case 4: regWZ++;                                        // increase memory pointer
    }
}

// PREFIX CB - switch to second instruction set
// If it is used without prefix IX or IY it only switches instruction set.
// It is is used with prefix IX or IY it is necessary to read offset byte (machine cycle 6) and folowed instruction op code (machine cycle 7)
// The machine cycle 7 is handled at individual instructions because the instruction set is changed during machine cycle 7
// and this function is not called, becuase it is from basic instruction set.
// M1 - set prefix CB and switch to machine cycle 6 in case of register IX or IY
// M6 - read offset from memory and calculate memory pointer WZ
void Z80Core::prefix_cb()
{
    switch(sm_MCycle) {
    case 1: mc_prefix = prefixCB;                           // set prefix flag
            if (XYState != rHL) {                           // test if it is used register IX or IY
                mc_StateMachine = sXYOffsetFetch;           // set address offset fetch flag
                readMem( m_PC++ );                            // next byte is read from memory, increase program counter PC
                mc_MCycles = 2;                             // set number of machine cycles to 2 to prevent change TStates in function runMCode()
            }
            break;
            // Address offset fetch
    case 6: mc_prefix = prefixCB;                           // set prefix flag
            readMem( m_PC );                                  // opcode followed offset is read from memory
            regWZ = regPairs[XYState] + static_cast<signed char>(sDI);   // calculation memory pointer
            mc_TStates = 5;                                 // set number of TStates for machine cycle 7
            break;
    }
}

// RLC r, RLC (HL), RLC (IX+d), RLC (IY+d), LD r,RLC (IX+d) and LD r,RLC (IY+d) - rotation left circular of register or byte at memory address stored in register pair
// RRC r, RRC (HL), RRC (IX+d), RRC (IY+d), LD r,RRC (IX+d) and LD r,RRC (IY+d) - rotation right circular of register or byte at memory address stored in register pair
// RL  r, RL  (HL), RL  (IX+d), RL  (IY+d), LD r,RL  (IX+d) and LD r,RL  (IY+d) - rotation left of register or byte at memory address stored in register pair
// RR  r, RR  (HL), RR  (IX+d), RR  (IY+d), LD r,RR  (IX+d) and LD r,RR  (IY+d) - rotation right of register or byte at memory address stored in register pair
// SLA r, SLA (HL), SLA (IX+d), SLA (IY+d), LD r,SLA (IX+d) and LD r,SLA (IY+d) - shift left aritmetic of register or byte at memory address stored in register pair
// SRA r, SRA (HL), SRA (IX+d), SRA (IY+d), LD r,SRA (IX+d) and LD r,SRA (IY+d) - shift right aritmetic of register or byte at memory address stored in register pair
// SLL r, SLL (HL), SLL (IX+d), SLL (IY+d), LD r,SLL (IX+d) and LD r,SLL (IY+d) - shift left logical of register or byte at memory address stored in register pair
// SRL r, SRL (HL), SRL (IX+d), SRL (IY+d), LD r,SRL (IX+d) and LD r,SRL (IY+d) - shift right logical of register or byte at memory address stored in register pair

// BIT b,r, BIT b,(HL), BIT b,(IX+d) and BIT b,(IY+d) - test bit of register or byte at memory address stored in register pair
// RES b,r, RES b,(HL), RES b,(IX+d), RES b,(IY+d), LD r,RES b,(IX+d) and LD r,RES b,(IY+d) - reset bit of register or byte at memory address stored in register pair
// SET b,r, SET b,(HL), SET b,(IX+d), SET b,(IY+d), LD r,SET b,(IX+d) and LD r,SET b,(IY+d) - set bit of register or byte at memory address stored in register pair

//s RLC r, RRC r, ... have only one machine cycle after prefix CB therefore only machine cycle 1 is executed
//
//s RLC (HL), RRC (HL), ... have three machine cycles after prefix CB therefore machine cycle from 1 to 3 are executed in sequence. The bit instrucion
// is one machine cycle shorter. The result is stored to register therefore it must be used some unused variable as function parameter.
//
// with prefix IX and IY have three machine cycles, but the address offset fetch are not after instruction itself, but after instruction PREFIX CB.
// In this case the sequence of machine cycles is machine cycle 1 of PREFIX CB, machine cycle 6 of PREFIX CB, machine cycle 7 of instruction, machine cycle 2
// of instruction and machine cycle 3 of instruction. The bit instrucion is one machine cycle shorter.  Machine cycle 1 of instruction is never executed.
// The result is stored to register therefore if it is not intended some unused variable must be used as function parameter.
void Z80Core::instCB( const uint8_t &ireg, uint8_t &reg )
{
    switch(sm_MCycle) {
                // Inicialization or doing selected operation with register
    case 1: if (mc_MCycles == 1) {                          // if instruction has only one machine cycle matematical operation is with register
                instCBOp( ireg, reg );
                if ( ( ireg & 0xc0 ) == 0x40 ) regF.copyYX( reg ); // instruction BIT
            } else {                                        // if instruction has more than one machine cycle matematical operation is with byte at memory address stored in register pair
                readMem( regPairs[rHL] );                 // next bus op. is read byte from memory
                mc_TStates = 4;                                 // set number of TStates for machine cycle 2
            }
            break;
            // Read byte from memory and do selected operation with byte at memory address stored in register pair
    case 2: instCBOp( ireg, sDI );
            if( ( ireg & 0xc0 ) == 0x40 ) { // instruction BIT
                regF.copyYX( regWZ.getHighByte() );
                finishInst();             // if the operation is bit test the last machine cycle is machine cycle 2
            } else {                                          // the operation except bit test has last machine cycle 3
                mc_MCycles = 3;                             // set number of machine cycles to 3
                if (XYState != rHL) writeMem( regWZ, sDI );  // test if it is used register IX or IY
                else writeMem( regPairs[rHL], sDI );  // set address for address bus
            }
            break;
            // Write byte to memory and to register
    case 3: reg = sDO;                                      // copy byte from memory also to register
            break;
            // New opcode fetch and program counter PC increase is in function clkRisingEdge()
            // Inicialization reading from memory; following machine cycle is machine cycle 2
    case 7: readMem( regWZ );                               // next bus op. is read byte from memory
            mc_TStates = 4;                                 // set number of TStates for machine cycle 2
    }
}

inline void Z80Core::instCBOp( const uint8_t &ireg, uint8_t &reg )
{
    switch ( ireg & 0xc0 ) {
    case 0x00: switch ( ireg & 0x38 ) {
               case 0x00: shiftOp< iRlc >( reg ); break;                              // rotation 1 bit left
               case 0x08: shiftOp< iRrc >( reg ); break;                              // rotation 1 bit right
               case 0x10: shiftOp< iRl >( reg ); break;                   // shift 1 bit left and copy flag C to bit 0
               case 0x18: shiftOp< iRr >( reg ); break;            // shift 1 bit right and copy flag C to bit 7
               case 0x20: shiftOp< iSla >( reg ); break;                                          // shift 1 bit left
               case 0x28: shiftOp< iSra >( reg ); break;                            // shift 1 bit right and keep bit 7
               case 0x30: shiftOp< iSll >( reg ); break;                                   // shift 1 bit left and set bit 0
               case 0x38: shiftOp< iSrl >( reg ); break;                                           // shift 1 bit right
               }
               break;
    case 0x40: regF.copySzeroZP( reg & mask( ireg ) ).setH().resetN(); break;   // test bit of register
    case 0x80: reg &= ~mask( ireg ); break;                                     // reset bit of register
    case 0xC0: reg |= mask( ireg ); break;                                      // set bit of register
    }

}

void Z80Core::in_r_imm( uint8_t &reg )// IN r,(N) - load register r from I/O address from byte after instruction
{
    switch(sm_MCycle) {
    case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and store it memory pointer
    case 2: regWZ = (reg << 8) | sDI;                       // set memory pointer
            readIO( regWZ );                                // next bus op. is read byte from I/O
            break;
    case 3: reg = sDI;                                      // store byte from I/O to register
            regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::in_r_rr( uint8_t &reg, const Z80RegPair &rp )// IN r,(rr) - load register r from I/O address stored in register pair
{
    switch(sm_MCycle) {
    case 1: regWZ = rp;                     // set memory pointer
            readIO( regWZ );                                // next bus op. is read byte from I/O
            break;
    case 2: reg = sDI;                                      // store byte from I/O to register
            regF.copySYXzeroZparityP( reg ).resetHN();
            regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::out_imm_r( const uint8_t &reg ) // OUT (N),r - store register r to I/O address from byte after instruction
{
    switch(sm_MCycle) {
    case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and store it memory pointer
    case 2: regWZ = (reg << 8) | sDI;                       // set memory pointer
            writeIO( regWZ, reg );                          // next bus op. is write byte to I/O
            break;
    case 3: regWZ.setLowByte( regWZ.getLowByte() + 1 );                                        // increase memory pointer
    }
}

void Z80Core::out_rr_r( const Z80RegPair &rp, const uint8_t &reg ) // OUT (rr),r - store register r to I/O address stored in register pair
{
    switch(sm_MCycle) {
    case 1: regWZ = rp;                     // set memory pointer
            writeIO( regWZ, reg );                          // next bus op. is write byte to I/O
            break;
    case 2: regWZ++;                                        // increase memory pointer
    }
}

void Z80Core::jr( const bool &cond )// JR N and JR cond,N - if condition cond is true then jump to relative address from byte after instruction
{
    switch( sm_MCycle ) {
    case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and if the condition is false then finish instruction
    case 2: if (cond) noBusOp( m_PC, 5 );                                     // if condition is true then jump to relative address otherwise the instruction is finished
            else finishInst();                             // set number of machine cycles to 2 - finish instruction
            break;
            // Calculation and set program counter PC
    case 3: regWZ = m_PC + static_cast<signed char>(sDI);         // calculation program counter
            jumpWZ();
    }
}

void Z80Core::jp( const bool &cond ) // JP NN and JP cond,NN - if condition cond is true then jump to address from bytes after instruction
{
    switch(sm_MCycle) {
    case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and store it to low byte of memory pointer
    case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
            readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and store it to high byte of memory pointer and if the condition is true change program counter PC
    case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
            if (cond) jumpWZ();
    }
}

void Z80Core::call( const bool &cond )// CALL NN and CALL cond,NN - if condition cond is true then store program counter PC to stack and jump to address from bytes after instruction
{
    switch(sm_MCycle) {
    case 1: readMem( m_PC++ ); break;                         // next bus op. is read byte from address PC, increase program counter PC
            // Read byte after instruction and store it to low byte of memory pointer
    case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
            readMem( m_PC++ );                                // next bus op. is read byte from address PC, increase program counter PC
            if (cond) mc_TStates = 4;                                 // set number of TStates for machine cycle 3
            break;
            // Read byte after instruction and store it to high byte of memory pointer and if the condition is false then finish instruction
    case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
            if (cond) pushStack8( m_PC >> 8 );                                     // if condition is true then store program counter PC to stack and jump to address otherwise the instruction is finished
            else finishInst();                             // set number of machine cycles to 3 - finish instruction
            break;
            // Write low byte of program counter PC to stack
    case 4: pushStack8( m_PC & 0xff ); break;                 // next bus op. is write byte to stack
            // Write high byte of program counter PC to stack and change program counter PC
    case 5: jumpWZ();
    }
}

void Z80Core::rst( const uint8_t &addr )//s RST NN - store PC to stack and jump to address NN
{
    switch(sm_MCycle) {
    case 1: pushStack8( m_PC >> 8 ); break;           // next bus op. is write byte to stack
            // Write high byte of program counter PC to stack
    case 2: pushStack8( m_PC & 0xff ); break;         // next bus op. is write byte to stack
            // Write low byte of program counter PC to stack and change program counter PC
    case 3: regWZ = addr;                                   // set memory pointer
            jumpWZ();
    }
}

void Z80Core::ret( const bool &cond ) //s RET and RET cond - if condition cond is true then load program counter PC from stack
{
    switch(sm_MCycle) {
    case 1: if (cond) popStack8();      // if condition is true then load program counter PC from stack otherwise the instruction is finished
            else finishInst();                            // set number of machine cycles to 1 - finish instruction
            break;
            // Read low byte of program counter PC from stack
    case 2: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
            popStack8();                                    // next bus op. is read byte from stack
            break;
            // Read high byte of program counter PC from stack
    case 3: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
            jumpWZ();
    }
}

void Z80Core::retn()//s RETI and RETN - load program counter PC from stack and copy flag IFF2 to flag IFF1
{
    ret( true );
    if (sm_MCycle == 3) IFF1 = IFF2;                                    // copy flag IFF2 to flag IFF1
}

// NOP during INT IM2 - When INT is actived and interrupt mode is set to 2 the interrupt vector is read from data bus and corresponding function is called
void Z80Core::int_im2()
{
    switch(sm_MCycle) {
            // Read interrupt vector and store it to memory pointer
    case 1: regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
            pushStack8( m_PC >> 8 );                  // next bus op. is write byte to stack
            break;
            // Write high byte of program counter PC to stack
    case 2: pushStack8( m_PC & 0xff );                // next bus op. is write byte to stack
            break;
            // Write low byte of program counter PC to stack and calculate addrese in interrupt vector table
    case 3: regWZ.setHighByte( regI );
            if (m_intVector == true) regWZ.setHighByte( 0xff );
            readMem( regWZ );                             // next bus op. is read byte from address PC, increase program counter PC
            regWZ++;
            break;
            // Read low byte from interrupt vector table
    case 4: readMem( regWZ );                                  // next bus op. is read byte from address PC
            regWZ.setLowByte( sDI );                 // low byte is stored to low byte of memory pointer
            break;
            // Read high byte from interrupt vector table and change program counter PC
    case 5: regWZ.setHighByte( sDI );          // high byte is stored to high byte of memory pointer
            jumpWZ();
    }
}

//s LDI, LDD, LDIR, LDDR - block transfers from memory to memory
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::ldxx()
{
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: regPairs[rBC]--;                     // decrease counter of bytes to transfer
            readMem( regPairs[rHL] );          // next bus op. is read byte from memory
            break;
            // Read byte from memory and increase/decrease register with source address
    case 2: if( type == bInc || type == bIncRep )           // if the instruction is LDI or LDIR
                regPairs[rHL]++;                 // increase source address
            if( type == bDec || type == bDecRep )           // if the instruction is LDD or LDDR
                regPairs[rHL]--;                 // decrease source address

            writeMem( regPairs[rDE], sDI );    // next bus op. is write byte to memory
            mc_TStates = 5;                                 // set number of TStates for machine cycle 3
            break;
            // Write byte to memory and increase/decrease register with destination address and check of termination of instruction
    case 3: if( type == bInc || type == bIncRep )          // if the instruction is LDI or LDIR
                regPairs[rDE]++;                 // increase destination address
            if( type == bDec || type == bDecRep )           // if the instruction is LDD or LDDR
                regPairs[rDE]--;                 // decrease destination address

            regF.copyBit1YX( sDI + regA ).resetHN().storeP( regPairs[rBC] != 0 );

            if ( regF.getP() ) {         // if counter of bytes to transfer is not zero then repeat instructions LDIR or LDDR or finish instruction LDI or LDD
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is LDIR or LDDR then continue exucuting instruction
                    noBusOp( m_PC, 5 );                          // next bus op. is no operation
                } else {                                    // if the instruction is LDI or LDD then finish instruction
//                    regF.copyBit1Y( sDI + regA ).resetHN().copyX( sDI + regA ).set( fP );
                }
            } else                                        // if counter of bytes to transfer is zero then finish instruction
                finishInst();                             // set number of machine cycles to 3 - finish instruction
            break;
            // Decrease program counter PC to first byte of this instrucion
    case 4: m_PC -= 2;                                     // set program counter PC back to this instruction
            regF.copyYX( m_PC >> 8 );
            regWZ = m_PC + 1;                              // set memory pointer
            sAO = m_PC;                                    // set address for address bus
    }
}

//s CPI, CPD, CPIR, CPDR - block comparison between memory and accumulator
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::cpxx()
{
    static uint8_t ioq;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: regPairs[rBC]--;                     // decrease counter of bytes to compare
            readMem( regPairs[rHL] );          // next bus op. is read byte from memory
            break;
            // Read byte from memory and compare it with accumulator and increase/decrease register with memory address
    case 2: mathOp< iCp, false >( regA, sDI );                                      // compare read byte and accumulator and set flags
            if( type == bInc || type == bIncRep ) {         // if the instruction is CPI or CPIR
                regPairs[rHL]++;                 // increase memory address for comparison
                regWZ++;                                    // increase memory pointer
            }
            if ( type == bDec || type == bDecRep ) {        // if the instruction is CPD or CPDR
                regPairs[rHL]--;                 // decrease memory address for comparison
                regWZ--;                                    // decrease memory pointer
            }
            noBusOp( m_PC, 5 );                                  // next bus op. is no operation
            break;
            // Check of termination of instruction
    case 3: ioq = regA - sDI - regF.getH();        // flags calculation
            regF.copyBit1YX( ioq ).storeP( regPairs[rBC] != 0 );

            if( !regF.getZ() && regF.getP()
             && ( type == bIncRep || type == bDecRep ) ) {  // if byte is not found and counter of bytes to compare is not zero then repeat instructions CPIR or CPDR or finish instruction CPI or CPD
                noBusOp( m_PC, 5 );                              // next bus op. is no operation
            } else finishInst();                          // if byte was found or counter of bytes to transfer is zero then finish instruction
            break;
            // Decrease program counter PC to first byte of this instrucion
    case 4: m_PC -= 2;                                     // set program counter PC back to this instruction
            regF.copyYX( m_PC >> 8 );
            regWZ = m_PC + 1;                              // set memory pointer
            sAO = m_PC;                                    // set address for address bus
    }
}

// INI, IND, INIR, INDR - block transfers from I/O address to memory
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::inxx()
{
    uint16_t ioq;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: regWZ = regPairs[rBC];             // set memory pointer
            readIO( regWZ );                                // next bus op. is read byte from I/O
            regs[rB]--;                                     // decrease counter of bytes to transfer
            break;
            // Read byte from I/O address
    case 2: writeMem( regPairs[rHL], sDI );    // next bus op. is write byte to memory
            break;
            // Write byte to memory, increase/decrease register with memory address and check of termination of instruction
    case 3: if( type == bInc || type == bIncRep ) {         // if the instruction is INI or INIR
                regPairs[rHL]++;                 // increase memory address for transfer
                regWZ++;                                    // increase memory pointer
                ioq = sDI + ( (regs[rC] + 1) & 0x00ff );      // calculation of variable ioq
            }
            if( type == bDec || type == bDecRep ) {         // if the instruction is IND or INDR
                regPairs[rHL]--;                 // decrease memory address to transfer
                regWZ--;                                    // decrease memory pointer
                ioq = sDI + ( (regs[rC] - 1) & 0x00ff );      // calculation of variable ioq
            }
            regF.copySYXzeroZ( regs[rB] ).storeHC( ioq >> 8 ).parityP( (ioq & 0x07) ^ regs[rB] ).copyBit7N( sDI );
            if( !regF.getZ() ) {                            // if counter of bytes to transfer is not zero then repeat instructions INIR or INDR or finish instruction INI or IND
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is INIR or INDR then continue exucuting instruction
                    noBusOp( m_PC, 5 );                          // next bus op. is no operation
                }
            } else finishInst();          // if counter of bytes to transfer is zero then finish instruction
            break;

            /// Repeated below
            // Decrease program counter PC to first byte of this instrucion
    case 4: m_PC -= 2;                                     // set PC back to this instruction
            regF.copyYX( m_PC >> 8 ).inotxxPH( sDI, regs[rB] );
            regWZ = m_PC + 1;                              // set memory pointer
            sAO = m_PC;                                    // set address for address bus
    }
}

// OUTI, OUTD, OTIR, OTDR - block transfers from memory to I/O address
// This function is a tempate with parameter bInc, bIncRep, bDec or bDecRep which selects increase or decrease of memory address and repeatibility
template<Z80Core::eBlock type>
inline void Z80Core::otxx()
{
    uint16_t ioq;
    switch(sm_MCycle) {
            // Inicialization and decrease counter of bytes to transfer
    case 1: readMem( regPairs[rHL] );          // next bus op. is read byte from memory
            regs[rB]--;                                     // decrease counter of bytes to transfer
            regWZ = regPairs[rBC];             // set memory pointer
            break;
            // Read byte from memory
    case 2: writeIO( regPairs[rBC], sDI );    // next bus op. is write byte to I/O
            break;
            // Write byte to I/O address, increase/decrease register with memory address and check of termination of instruction
    case 3: if ( type == bInc || type == bIncRep ) {    // if the instruction is OUTI or OTIR
                regPairs[rHL]++;                 // increase memory address for transfer
                regWZ++;                                    // increase memory pointer
            }
            if ( type == bDec || type == bDecRep ) {    // if the instruction is OUTD or OTDR
                regPairs[rHL]--;                 // decrease memory address for transfer
                regWZ--;                                    // decrease memory pointer
            }
            ioq = sDI + regs[rL];                           // calculation of variable ioq
            regF.copySYXzeroZ( regs[rB] ).storeHC( ioq >> 8 ).parityP( (ioq & 0x07) ^ regs[rB] ).copyBit7N( sDI );
            if ( !regF.getZ() ) {                            // if counter of bytes to transfer is not zero then repeat instructions OTIR or OTDR or finish instruction OUTI or OUTD
                if( type == bIncRep || type == bDecRep ) {  // if the instruction is OTIR or OTDR then continue exucuting instruction
                    noBusOp( m_PC, 5 );                          // next bus op. is no operation
                }
            } else finishInst();          // if counter of bytes to transfer is zero then finish instruction
            break;

            /// Repeated above
            // Decrease program counter PC to first byte of this instrucion
    case 4: m_PC -= 2;                                     // set program counter PC back to this instruction
            regF.copyYX( m_PC >> 8 ).inotxxPH( sDI, regs[rB] );
            regWZ = m_PC + 1;                              // set memory pointer
            sAO = m_PC;                                    // set address for address bus
    }
}

void Z80Core::setProducer( QString producer )
{
    int p = m_enumUids.indexOf( producer ); // getEnumIndex( producer );
    m_producer = (eProducer)p;
}
void Z80Core::setCmos( bool cmos ) { m_cmos = cmos; }// Setter for CMOS or NMOS version
void Z80Core::setIoWait( bool ioWait ) { m_ioWait = !ioWait; } // Setter for single wait I/O operation
void Z80Core::setIntVector( bool intVector ) { m_intVector = intVector; } // Setter for force interrupt vector 0xff
