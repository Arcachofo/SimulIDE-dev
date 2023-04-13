/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "i51core.h"
#include "simulator.h"
#include "mcuport.h"
#include "mcupin.h"

#define ACC  m_acc[0]
#define BANK ( (STATUS(RS0)>>RS0) | (STATUS(RS1)>>RS1) ) //( (PSW & (PSWMASK_RS0|PSWMASK_RS1))>>PSW_RS0 )
#define I_RX_VAL ( m_dataMem[(m_opcode & 1) + 8*BANK] )
#define RX_ADDR ( (m_opcode & 7) + 8*BANK )

I51Core::I51Core( eMcu* mcu  )
       : McuCpu( mcu  )
       , eElement( mcu->getId()+"-I51Core" )
{
    m_eaPin = mcu->getIoPin("EA");
    //m_rwPin = mcu->getPin("RW");
    //m_rePin = mcu->getPin("EA");
    m_enPin = mcu->getIoPin("PSEN");
    m_laPin = mcu->getIoPin("ALE");

    m_dataPort = mcu->getMcuPort("PORT0");
    m_addrPort = mcu->getMcuPort("PORT2");

    m_acc = m_mcu->getReg( "ACC" );

    m_upperData = (m_dataMemEnd > m_regEnd);
}
I51Core::~I51Core() {}

void I51Core::stamp()
{
}

void I51Core::reset()
{
    CpuBase::reset();
    m_tmpPC = 0;
    m_cycle = 0;
    m_cpuState = cpu_RESET;

    m_extPGM = false;

    Simulator::self()->cancelEvents( this );
    //m_memState = mem_IDLE;
    //m_read = true;

    /*if( m_rwPin ){
        m_rwPin->controlPin( true, true );
        m_rwPin->setPinMode( output );
        m_rwPin->setOutState( true );
    }
    if( m_rePin ) m_rePin->sheduleState( true, 0 );*/

    m_enPin->setPinMode( output );
    m_enPin->setOutState( true );
    m_enPin->updateStep();

    m_laPin->setPinMode( output );
    m_laPin->setOutState( false );
    m_laPin->updateStep();

    m_psStep = m_mcu->psCycle()/12;  // We are doing 2 Read cycles per Machine cycle
    m_addrSetTime = 3*m_psStep;    // Addr pins
    m_laEnEndTime = 4*m_psStep;    // LA 0
    m_readSetTime = 6*m_psStep;    // PSEN 0
    // m_writeSetTime ???
    m_readBusTime = 12*m_psStep-10; // PSEN 1, Read data Bus
}

void I51Core::runEvent()
{
    switch( m_memState ) {
        case mem_IDLE: break;
        case mem_LAEN:            // Enable Latch for Low addr
        {
            //m_laPin->sheduleState( true, 0 );
            m_laPin->setOutState( true );
            Simulator::self()->addEvent( m_addrSetTime, this );
            m_memState = mem_ADDR;
        }break;
        case mem_ADDR:            // Set Address Bus
        {
            //if( m_readMode & RW ) m_rwPin->sheduleState( m_read, 0 ); // Set RW Pin Hi/Lo

            // We are latching Low Address byte in Data Bus
            m_dataPort->setOutState( m_addr ); // Low  addr byte to Data Pins
            m_addrPort->setOutState( m_addrH );// High addr byte to Addr Pins

            Simulator::self()->addEvent( m_laEnEndTime - m_addrSetTime, this );
            m_memState = mem_LADI;
        } break;
        case mem_LADI:           // Disable Latch
        {
            //m_laPin->sheduleState( false, 0 );
            m_laPin->setOutState( false );

            m_dataTime = m_read ? m_readSetTime : m_writeSetTime;
            uint64_t time = m_dataTime -m_laEnEndTime;
            Simulator::self()->addEvent( time, this );
            m_memState = mem_DATA;
        }break;
        case mem_DATA:           // Set Data Bus for Read or Write
        {
            /// if( m_readMode & RW ) m_rwPin->setOutState( m_read ); // Set RW Pin Hi/Lo

            if( m_read )
            {
                m_dataPort->setOutState( 0xFF ); // Input
                m_enPin->sheduleState( false, 1 ); // Set EN  Pin Low
                uint64_t time = m_readBusTime - m_dataTime;

                Simulator::self()->addEvent( time, this );
                m_memState = mem_READ;
            }else{
                m_dataPort->setOutState( m_data );// setDataBus( m_data ); // Set data Pins States
                m_memState = mem_IDLE;
            }
        } break;
        case mem_READ:           // Read Data Bus
        {
            m_data = m_dataPort->getInpState();
            m_enPin->setOutState( true ); // Set EN  Pin High
            m_memState = mem_IDLE;
        } break;
    }
}

void I51Core::runStep()
{
    m_mcu->cyclesDone = 1;
    m_cycle++;

    bool extPGM = !m_eaPin->getInpState();
    if( m_extPGM != extPGM )
    {
        m_dataPort->controlPort( extPGM, extPGM );
        m_addrPort->controlPort( extPGM, extPGM );
        m_extPGM = extPGM;
    }

    if( extPGM ) m_pgmData = m_data;             // Read from External
    else         m_pgmData = m_progMem[m_tmpPC]; // Read from Internal ROM

    if( m_cpuState == cpu_FETCH )     // Read cycle 1
    {
        m_opcode = m_pgmData;
        m_cycle = 0;
        m_tmpPC++;
        Decode();
        m_cpuState = cpu_OPERAND;
    }
    else if( m_cpuState == cpu_OPERAND ) // Read cycle > 1
    {
        if( !m_dataEvent.isEmpty() ) // Read next operand
        {
            readOperand();
            m_tmpPC++;
        }
        else m_cpuState = cpu_EXEC; // All operands ready
    }
    else if( m_cpuState == cpu_RESET ) m_cpuState = cpu_FETCH; // First cycle used fetching first instruction

    if( m_cpuState == cpu_EXEC )  // Execute Instruction
    {
        m_PC = m_tmpPC;
        Exec();
        m_tmpPC = m_PC;
        m_cpuState = cpu_FETCH;
    }
    if( extPGM )// m_mcu->extMem->read( m_tmpPC, ExtMemModule::EN | ExtMemModule::LA );
    {
        m_read = true;
        m_addr = m_tmpPC;
        m_addrH = m_tmpPC >> 8;
        m_memState = mem_LAEN;
        runEvent();//Start next Read cycle
    }
}

void I51Core::readOperand()
{
    uint8_t addrMode = m_dataEvent.takeFirst();

    if( addrMode & aIMME ){
        if( addrMode & aORIG ) m_op0 = m_opAddr = m_pgmData;
        else{
            if     ( m_cycle == 1 ) m_opAddr = (uint16_t)m_pgmData << 8; /// 16 bit addrs
            else if( m_cycle == 2 ) m_opAddr |= m_pgmData;
        }
    }
    else if( addrMode & aDIRE ){
        if( addrMode & aORIG ) m_op0 = GET_RAM( m_pgmData );
        else                   m_opAddr = m_pgmData;
    }
    else if( addrMode & aRELA ){
        if( addrMode & aORIG ) m_op2 = m_pgmData;
        else                   m_opAddr = m_pgmData;
    }
    else if( addrMode & aBIT ){   // Get bit mask an Reg address
        m_bitAddr = m_pgmData;
        m_bitMask = 1 << (m_pgmData & 7);
        if( m_bitAddr > m_lowDataMemEnd ) m_bitAddr &= 0xF8;
        else{ m_bitAddr >>= 3; m_bitAddr += 0x20; }
    }
}

void I51Core::operRgx() { m_op0 = m_RxAddr; }//{ m_op0 = m_dataMem[m_RxAddr]; }
void I51Core::operInd() { m_op0 = m_dataMem[ checkAddr( I_RX_VAL ) ]; }
void I51Core::operI08() { m_dataEvent.append( aIMME | aORIG | aPGM ); }     // m_op0 = data
void I51Core::operDir() { m_dataEvent.append( aDIRE | aORIG | aPGM ); }     // m_op0 = GET_RAM( data );
void I51Core::operRel() { m_dataEvent.append( aRELA | aORIG | aPGM );       // m_op2 = data;
                          m_dataEvent.append( aRELA         | aPGM ); }     // m_offset = data;

void I51Core::addrRgx() { m_opAddr = m_RxAddr; }
void I51Core::addrInd() { m_opAddr = checkAddr( I_RX_VAL );}             //
void I51Core::addrI08() { m_dataEvent.append( aIMME | aORIG ); }            // m_opAddr = data;
void I51Core::addrDir() { m_dataEvent.append( aDIRE | aPGM ); }            // m_opAddr = data;
void I51Core::addrBit( bool invert ) { m_dataEvent.append( aBIT  | aPGM ); // m_opAddr = addr, m_op0 = bitMask
                                       m_invert = invert; }

void I51Core::pushStack8( uint8_t value )
{
    REG_SPL++;
    uint16_t address = checkAddr( REG_SPL );
    m_dataMem[address] = value;
}

uint8_t I51Core::popStack8()
{
    uint16_t address = checkAddr( REG_SPL );
    uint8_t value = m_dataMem[address];
    REG_SPL--;
    return value;
}

void I51Core::addFlags( uint8_t value1, uint8_t value2, uint8_t acc )
{
    uint8_t c = ((value1+value2+acc)>>1) & 1<<7;
    write_S_Bit( Cy, c );                                                  // Carry: overflow from 7th bit to 8th bit
    write_S_Bit( AC, ((value1 & 0x0F)+(value2 & 0x0F) + acc ) & 1<<4 );    // Auxiliary carry: overflow from 3th bit to 4th bit
    write_S_Bit( OV, (((value1 & 127)+(value2 & 127 ) + acc )^c ) & 1<<7); // Overflow: overflow from 6th or 7th bit, but not both
}

void I51Core::subFlags( uint8_t value1, uint8_t value2 )
{
    uint8_t c = ((value1-value2)>>1) & 1<<7; //Carry: overflow from 7th bit to 8th bit
    write_S_Bit( Cy, c );                                             // Carry: overflow from 7th bit to 8th bit
    write_S_Bit( AC, ((value1 & 0x0F)-(value2 & 0x0F)) & 1<<4 );
    write_S_Bit( OV, (((value1 & 127)-(value2 & 127)) ^ c ) & 1<<7);
}

// INSTRUCTIONS -----------------------------

void I51Core::AJMP() { m_PC = ((m_PC+2) & 0xF800) | m_opAddr | (( m_opcode & 0xE0 ) << 3 ); }
void I51Core::LJMP() { m_PC = m_opAddr; }
void I51Core::JMP()  { m_PC = GET_REG16_LH( REG_DPL ) + ACC; }
void I51Core::SJMP() { m_PC += (int8_t)m_opAddr; }

void I51Core::ACALL()
{
    pushStack8( m_PC & 0xFF );
    pushStack8( m_PC >> 8 );
    AJMP();
}

void I51Core::RR() { ACC = (ACC >> 1) | (ACC << 7); }
void I51Core::RL() { ACC = (ACC << 1) | (ACC >> 7); }

void I51Core::RRC()
{
    uint8_t c = STATUS(Cy);
    write_S_Bit( Cy, ACC & 1 );
    ACC = (ACC >> 1) | c; // STATUS carry is already in bit 7
}

void I51Core::RLC()
{
    uint8_t newc = ACC & (1<<7);
    ACC = ACC << 1;
    if( STATUS(Cy) ) ACC += 1;
    write_S_Bit( Cy, newc );
}

void I51Core::LCALL()
{
    pushStack8(( m_PC+3 ) & 0xFF );
    pushStack8(( m_PC+3 ) >> 8 );
    m_PC = m_opAddr;
}

void I51Core::RET() { m_PC  = popStack8() << 8; m_PC |= popStack8(); }

void I51Core::JBC()
{
    if( m_dataMem[m_bitAddr] & m_bitMask )
    {
        m_dataMem[m_bitAddr] &= ~m_bitMask;
        m_PC += (int8_t)m_opAddr;
    }
}

void I51Core::JB()
{ if(   m_dataMem[m_bitAddr] & m_bitMask  ) m_PC += (int8_t)m_opAddr; }

void I51Core::JNB()
{ if( !(m_dataMem[m_bitAddr] & m_bitMask) ) m_PC += (int8_t)m_opAddr; }

void I51Core::JC()  { if(  STATUS(Cy) ) m_PC += (int8_t)m_opAddr; }
void I51Core::JNC() { if( !STATUS(Cy) ) m_PC += (int8_t)m_opAddr; }
void I51Core::JZ()  { if( !ACC        ) m_PC += (int8_t)m_opAddr; }
void I51Core::JNZ() { if(  ACC        ) m_PC += (int8_t)m_opAddr; }

void I51Core::MOVbc()
{
    uint8_t carry = STATUS(Cy) >> Cy;
    uint8_t     value = m_dataMem[m_bitAddr] & ~m_bitMask; // Clear bit
    if( carry ) value = m_dataMem[m_bitAddr] | m_bitMask;  // Set bit if Carry

    SET_RAM( m_bitAddr, value );
}
void I51Core::MOVc()
{
    uint8_t value = (m_dataMem[m_bitAddr] & m_bitMask) ? 1 : 0;
    write_S_Bit( Cy, value );
}

void I51Core::ORLc()
{
    uint8_t carry = STATUS(Cy) >> Cy;

    uint8_t value = m_dataMem[m_bitAddr] & m_bitMask;
    if( m_invert ) value = value ? 1 : carry;
    else           value = value ? carry : 1;

    write_S_Bit( Cy, value );
}

void I51Core::ANLc()
{
    uint8_t carry = STATUS(Cy) >> Cy;

    uint8_t value = m_dataMem[m_bitAddr] & m_bitMask ;
    if( m_invert ) value = value ? 0 : carry;
    else           value = value ? carry : 0;

    write_S_Bit( Cy, value );
}

void I51Core::CLRc()  { clear_S_Bit( Cy ); }
void I51Core::SETBc() { set_S_Bit( Cy ); }
void I51Core::CPLc()  { *m_STATUS ^= 1 << Cy; }

void I51Core::CLRb()  { SET_RAM( m_bitAddr, m_dataMem[m_bitAddr] & ~m_bitMask ); }
void I51Core::SETBb() { SET_RAM( m_bitAddr, m_dataMem[m_bitAddr] | m_bitMask ); }
void I51Core::CPLb()  { SET_RAM( m_bitAddr, m_dataMem[m_bitAddr] ^ m_bitMask ); }

void I51Core::CJNE()  ///
{
    write_S_Bit( Cy, m_op0 < m_op2 );
    if( m_op0 != m_op2 ) m_PC += m_opAddr;
}

void I51Core::DJNZ()
{
    int value = m_dataMem[m_op0]-1; // m_op0 = Rx or Dir
    SET_RAM( m_op0, value );
    if( value ) m_PC += (int8_t)m_opAddr;
}

void I51Core::PUSH() { pushStack8( m_op0 ); }
void I51Core::POP()  { SET_RAM( m_opAddr, popStack8() ); }

void I51Core::CLRa()  { ACC = 0; }
void I51Core::CPLa()  { ACC = ~ACC; }
void I51Core::SWAPa() { ACC = (ACC << 4) | (ACC >> 4); }
void I51Core::DAa()
{
    uint8_t AL = ACC & 0x0F;
    if( AL > 0x09 || STATUS(AC) ){
        AL  += 6;
        ACC += 6;
        if( AL & 1<<4 ) set_S_Bit( Cy );
    }
    uint8_t AH = (ACC & 0xF0) >> 4;
    if( AH > 0x09 || STATUS(AC) ){
        AH += 0x06;
        if( AH & 1<<4 ) set_S_Bit( Cy );
        ACC = AL | (AH << 4);
    }
}

void I51Core::INCd()
{
    //SET_REG16_LH( GET_REG16_LH( REG_DPL ) + 1);
    SET_RAM( REG_DPL, m_dataMem[REG_DPL]+1);
    if( !m_dataMem[REG_DPL] ) SET_RAM( REG_DPH, m_dataMem[REG_DPH]+1 );
}
void I51Core::INC() { m_dataMem[m_opAddr]++; }
void I51Core::DEC() { m_dataMem[m_opAddr]--; }

void I51Core::ADD() { addFlags( m_op0, ACC, 0 ); ACC += m_op0; }
void I51Core::ADDC()
{
    uint8_t carry = STATUS(Cy) >> Cy;
    addFlags( m_op0, ACC, carry );
    ACC += m_op0 + carry;
}
void I51Core::ORLm() { SET_RAM( m_opAddr, m_dataMem[m_opAddr] | m_op0 ); }
void I51Core::ANLm() { SET_RAM( m_opAddr, m_dataMem[m_opAddr] & m_op0 ); }
void I51Core::XRLm() { SET_RAM( m_opAddr, m_dataMem[m_opAddr] ^ m_op0 ); }

void I51Core::ORLa() { ACC |= m_op0; }
void I51Core::ANLa() { ACC &= m_op0; }
void I51Core::XRLa() { ACC ^= m_op0; }

void I51Core::SUBB()
{
    if( STATUS(Cy) ) m_op0--;
    subFlags( ACC, m_op0 );
    ACC -= m_op0;
}

void I51Core::XCH() //
{
    uint8_t a = ACC ;
    ACC = m_dataMem[m_opAddr];
    SET_RAM( m_opAddr, a );
}

void I51Core::XCHD()
{
    uint8_t value = m_dataMem[m_opAddr];
    m_dataMem[m_opAddr] = (value & 0xF0) | (ACC & 0x0F);
    ACC = (ACC & 0xF0) | (value & 0x0F);
}

void I51Core::DIVab()
{
    uint A = ACC;
    uint B = m_dataMem[REG_B];

    if( B ){
        ACC = A/B;
        m_dataMem[REG_B] = A % B;
        clear_S_Bit( OV );
    }
    else set_S_Bit( OV );

    clear_S_Bit( Cy );
}

void I51Core::MULab()
{
    uint res = ACC*m_dataMem[REG_B];
    ACC = res & 0xFF;
    m_dataMem[REG_B] = res >> 8;

    write_S_Bit( OV, m_dataMem[REG_B] );
    clear_S_Bit( Cy );
}

void I51Core::MOVr()  { m_dataMem[m_opAddr] = m_op0; }
void I51Core::MOVm()  { SET_RAM( m_opAddr, m_op0 ); }
void I51Core::MOVa()  { ACC = m_op0; }
void I51Core::MOVCd() { ACC = m_progMem[ GET_REG16_LH( REG_DPL ) + ACC ]; }
void I51Core::MOVCp() { ACC = m_progMem[ m_PC + 1 + ACC ]; }
void I51Core::MOVd()  { SET_REG16_LH( REG_DPL, m_opAddr ); }

void I51Core::movx_a_indir_dptr()
{
    /*int dptr =( m_dataMem[REG_DPH] << 8 ) | m_dataMem[REG_DPL];
    if       ( aCPU->xread )    ACC = aCPU->xread(dptr );
    else { if( aCPU->mExtData ) ACC = aCPU->mExtData[dptr &( aCPU->mExtDataSize - 1 )]; }*/
}

void I51Core::movx_a_indir_rx()
{
    /*int address = I_RX_VAL;
    if       ( aCPU->xread )    ACC = aCPU->xread(address );
    else { if( aCPU->mExtData ) ACC = aCPU->mExtData[address &( aCPU->mExtDataSize - 1 )]; }*/
}

void I51Core::movx_indir_dptr_a()
{
    /*int dptr =( m_dataMem[REG_DPH] << 8 ) | m_dataMem[REG_DPL];
    if       ( aCPU->xwrite ) aCPU->xwrite(dptr, ACC );
    else { if( aCPU->mExtData ) aCPU->mExtData[dptr &( aCPU->mExtDataSize - 1 )] = ACC; }*/
}

void I51Core::movx_indir_rx_a()
{
    /*int address = I_RX_VAL;

    if( aCPU->xwrite ) aCPU->xwrite(address, ACC );
    else { if( aCPU->mExtData ) aCPU->mExtData[address &( aCPU->mExtDataSize - 1 )] = ACC; }*/
}

void I51Core::Decode()
{
    m_dataEvent.clear();
    if( m_opcode & 8 ) // Rx
    {
        uint8_t nibbleH = (m_opcode & 0xF0) >> 4;
        m_RxAddr        = (m_opcode & 0x07) + 8*BANK;

        switch( nibbleH ) {                           // b-c Inst
            case 0x00:                                // 1-1 INC  Rx <- R++
            case 0x01: addrRgx();            break;   // 1-1 DEC  Rx <- R--
            case 0x02:                                // 1-1 ADD  Ac += Rx
            case 0x03:                                // 1-1 ADDC Ac += Rx+C
            case 0x04:                                // 1-1 ORL  Ac |= Rx
            case 0x05:                                // 1-1 ANL  Ac &= Rx
            case 0x06:            operRgx(); break;   // 1-1 XRL  Ac ^= Rx
            case 0x07: addrRgx(); operI08(); break;   // 2-1 MOVr Rx <- #imm8
            case 0x08: addrDir(); operRgx(); break;   // 2-2 MOVm Di <- Rx
            case 0x09:            operRgx(); break;   // 1-1 SUBB Ac -= Rx-C
            case 0x0A: addrRgx(); operDir(); break;   // 2-2 MOVr Rx <- Di
            case 0x0B: operRgx(); operRel(); break;   // 3-2 CJNE Rx == Op1 ? 0 :Jump
            case 0x0C: addrRgx();            break;   // 1-1 XCH  Ac <> Rx
            case 0x0D: operRgx(); addrDir(); break;   // 2-2 DJNZ Rx--      ? Jump : 0
            case 0x0E:            operRgx(); break;   // 1-1 MOVa Ac <- Rx
            case 0x0F: break; //m_op0=ACC; addrRgx(); break; // 1 MOVX Rx, A
        }
    }else{
        switch( m_opcode ){                           // b-c Inst
            case 0x00:                       break;   // 1-1 NOP
            case 0x01: addrI08();            break;   // 2-2 AJMP addr11
            case 0x02: addrI08(); addrI08(); break;   // 3-2 LJMP addr16
            case 0x03:                       break;   // 1-1 RRa Ac >> 1
            case 0x04:                       break;   // 1-1 INC Ac <- Ac++  Accumulator
            case 0x05: addrDir();            break;   // 2-1 INC Di <- Di++  Direct
            case 0x06:
            case 0x07: addrInd();            break;   // 1-1 INC In <- In++  @Indirect Rx

            case 0x10: addrBit(); addrI08(); break;   // 3-2 JBC
            case 0x11: addrI08();            break;   // 2-2 ACALL addr11
            case 0x12: addrI08(); addrI08(); break;   // 3-2 LCALL addr16
            case 0x13:                       break;   // 1-1 RRCa C >> Ac>>1
            case 0x14:                       break;   // 1-1 DEC Ac <- Ac++  Accumulator
            case 0x15: addrDir();            break;   // 2-1 DEC Di <- Di++  Direct
            case 0x16:
            case 0x17: addrInd();            break;   // 1-1 DEC In <- In++  @Indirect Rx

            case 0x20: addrBit(); addrI08(); break;   // 3-2 JB
            case 0x21: addrI08();            break;   // 2-2 AJMP addr11
            case 0x22:                       break;   // 1-2 RET
            case 0x23:                       break;   // 1-1 RLa Ac << 1
            case 0x24:            operI08(); break;   // 2-1 ADD Ac += #imm8  #Imminent
            case 0x25:            operDir(); break;   // 2-1 ADD AC += Di     Direct
            case 0x26:
            case 0x27:            operInd(); break;   // 1-1 ADD Ac += In     @Indirect

            case 0x30: addrBit(); addrI08(); break;   // 2-2 JNB
            case 0x31: addrI08();            break;   // 2-2 ACALL addr11
            case 0x32:                       break;   // 1-2 RETI
            case 0x33:                       break;   // 1-1 RLCa C << Ac<<1
            case 0x34:            operI08(); break;   // 2-1 ADDC Ac += C+#imm8  #Imminent
            case 0x35:            operDir(); break;   // 2-1 ADDC AC += C+Di     Direct
            case 0x36:
            case 0x37:            operInd(); break;   // 1-1 ADDC Ac += C+In     @Indirect

            case 0x40: addrI08();            break;   // 2-2 JC C ? jump : 0
            case 0x41: addrI08();            break;   // 2-2 AJMP addr11
            case 0x42: addrDir(); m_op0=ACC; break;   // 2-1 ORL Di |= Ac
            case 0x43: addrDir(); operI08(); break;   // 3-2 ORL Di |= #I8
            case 0x44:            operI08(); break;   // 2-1 ORL Ac |= #I8   #Imminent
            case 0x45:            operDir(); break;   // 2-1 ORL AC |= Di    Direct
            case 0x46:
            case 0x47:            operInd(); break;   // 1-1 ORL Ac |= In    @Indirect

            case 0x50: addrI08();            break;   // 2-2 JNC C ? 0 : jump
            case 0x51: addrI08();            break;   // 2-2 ACALL addr11
            case 0x52: addrDir(); m_op0=ACC; break;   // 2-1 ANL Di &= Ac
            case 0x53: addrDir(); operI08(); break;   // 3-2 ANL Di &= #I8
            case 0x54:            operI08(); break;   // 2-1 ANL Ac &= #I8 #Imminent
            case 0x55:            operDir(); break;   // 2-1 ANL Ac &= Di Direct
            case 0x56:
            case 0x57:            operInd(); break;   // 1-1 ANL Ac &= In

            case 0x60: addrI08();            break;   // 2-2 JZ A == 0 ? jump : 0
            case 0x61: addrI08();            break;   // 2-2 AJMP addr11
            case 0x62: addrDir(); m_op0=ACC; break;   // 2-1 XRL  Di ^= Ac
            case 0x63: addrDir(); operI08(); break;   // 3-2 XRL  Di ^= #I8
            case 0x64:            operI08(); break;   // 2-1 XRLa Ac ^= #I8 #Imminent
            case 0x65:            operDir(); break;   // 2-1 XRLa Ac ^= Di  Direct
            case 0x66:
            case 0x67:            operInd(); break;   // 1-1 XRLa Ac ^= In  @Indirect

            case 0x70: addrI08();            break;   // 2-2 JNZ A == 0 ? 0 : jump
            case 0x71: addrI08();            break;   // 2-2 ACALL addr11
            case 0x72: addrBit();            break;   // 2-2 ORLc  C  <- C|b
            case 0x73:                       break;   // 1-2 JMP  Dptr+A
            case 0x74:            operI08(); break;   // 2-1 MOVa Ac <- #I8  #Imminent
            case 0x75: addrDir(); operI08(); break;   // 3-2 MOVm Di <- #I8
            case 0x76:
            case 0x77: addrInd(); operI08(); break;   // 2-1 MOVm In <- #I8  @Indirect

            case 0x80: addrI08();            break;   // 2-2 SJMP PC += addr8
            case 0x81: addrI08();            break;   // 2-2 AJMP addr11
            case 0x82: addrBit();            break;   // 2-2 ANLc  C  <- C&b
            case 0x83:                       break;   // 1-2 MOVCp Ac += (PC)
            case 0x84:                       break;   // 1-4 DIVab Ac <- Ac/B
            case 0x85: addrDir(); operDir(); break;   // 3-2 MOVm  Di <- Di
            case 0x86:
            case 0x87: addrDir(); operInd(); break;   // 2-2 MOVm Di <- In  @Indirect

            case 0x90: addrI08(); addrI08(); break;   // 3-2 MOVd  Dptr <- addr16;
            case 0x91: addrI08();            break;   // 2-2 ACALL addr11
            case 0x92: addrBit();            break;   // 2-2 MOVbc b  <- C
            case 0x93:                       break;   // 1-2 MOVCd Ac += (Dptr)
            case 0x94:            operI08(); break;   // 2-1 SUBB  Ac -= #I8 #Imminent
            case 0x95:            operDir(); break;   // 2-1 SUBB  Ac -= Di  Direct
            case 0x96:
            case 0x97:            operInd(); break;   // 1-1 SUBB Ac -= In  @Indirect

            case 0xa0: addrBit( true );      break;   // 2-2 ORLc  C <- C|!b
            case 0xa1: addrI08();            break;   // 2-2 AJMP addr11
            case 0xa2: addrBit();            break;   // 2-1 MOVc C <- b
            case 0xa3:                       break;   // 1-2 INCd Dptr++
            case 0xa4:                       break;   // 1-4 MULab A <- A*B
            case 0xa5:                       break;   // Nop unused
            case 0xa6:
            case 0xa7: addrInd(); operDir(); break;   // 2-2 MOVm In <- Di @Indirect

            case 0xb0: addrBit( true );      break;   // 2-2 ANLc  C  <- C&!b
            case 0xb1: addrI08();            break;   // 2-2 ACALL addr11
            case 0xb2: addrBit();            break;   // 2-1 CPLb b = !b
            case 0xb3:                       break;   // 2-1 CPLc C = !C
            case 0xb4: m_op0=ACC; operRel(); break;   // 3-2 CJNE Ac == #in8 ? 0 :Jump
            case 0xb5: m_op0=ACC; operRel(); break;   // 3-2 CJNE Ac == Di   ? 0 :Jump
            case 0xb6:
            case 0xb7: operInd(); operRel(); break;   // 3-2 CJNE In == #in8 ? 0 :Jump

            case 0xc0:            operDir(); break;   // 2-2 PUSH Stack <- Di
            case 0xc1: addrI08();            break;   // 2-2 AJMP addr11
            case 0xc2: addrBit();            break;   // 2-2 CLRb b = 0
            case 0xc3:                       break;   // 1-1 CLRc C = 0
            case 0xc4:                       break;   // 1-1 SWAPa
            case 0xc5: addrDir();            break;   // 1-1 XCH Ac <> Di  Direct
            case 0xc6:
            case 0xc7: addrInd();            break;  // 1-1 XCH Ac <> In @Indirect

            case 0xd0: addrDir();            break;   // 2-2 POP  Di <- Stack
            case 0xd1: addrI08();            break;   // 2-2 ACALL addr11
            case 0xd2: addrBit();            break;   // 2-2 SETBb b = 1
            case 0xd3:                       break;   // 1-1 SETBc C = 1
            case 0xd4:                       break;   // 1-1 DAa
            case 0xd5: operDir(); addrDir(); break;   // 3-2 DJNZ Dir-- ? Jump : 0
            case 0xd6:
            case 0xd7: addrInd();            break;   //1-1 XCHD A In @Indirect

            case 0xe0: // movx_a_indir_dptr(); break;
            case 0xe1: addrI08();            break;   // 2-2 AJMP addr11
            case 0xe2: // movx_a_indir_rx(); break;
            case 0xe3: // movx_a_indir_rx(); break;
            case 0xe4:                       break;   // 1-1 CLR A
            case 0xe5:            operDir(); break;   // 2-1 MOVa A <- Di  Direct
            case 0xe6:
            case 0xe7:            operInd(); break;   // 1-1 MOV In <- Ac  @Indirect Rx

            case 0xf0: // movx_indir_dptr_a(); break;
            case 0xf1: addrI08();            break;   // 2-2 ACALL addr11
            case 0xf2: // movx_indir_rx_a(); break;
            case 0xf3: // movx_indir_rx_a(); break;
            case 0xf4:                       break;
            case 0xf5: /// m_op0 = ACC; operI08(); break;
            case 0xf6:
            case 0xf7: break; /// m_op0 = ACC; addrInd(); MOVX(); break;  // @Indirect Rx
       }
    }
}

void I51Core::Exec()
{
    if( m_opcode & 8 ) //Rx
    {
        //uint8_t nibbleL = m_opcode & 0x07;
        uint8_t nibbleH = (m_opcode & 0xF0) >> 4;
        m_RxAddr        = (m_opcode & 0x07) + 8*BANK;

        switch( nibbleH ) {
            case 0x00: INC();   break;   // Rx
            case 0x01: DEC();   break;   // Rx
            case 0x02: ADD();   break;   // Rx
            case 0x03: ADDC();  break;   // Rx
            case 0x04: ORLa();  break;   // Rx
            case 0x05: ANLa();  break;   // Rx
            case 0x06: XRLa();  break;   // Rx
            case 0x07: MOVr();  break;   // Rx, #imm
            case 0x08: MOVm();  break;   // Rx, Direct
            case 0x09: SUBB();  break;   // Rx
            case 0x0A: MOVr();  break;   // Rx, Direct
            case 0x0B: CJNE();  break;   // Rx, Rel
            case 0x0C: XCH();   break;   // Rx
            case 0x0D: DJNZ();  break;   // DJNZ Rx, rel
            case 0x0E: MOVa();  break;   // MOV A, Rx
            case 0x0F: break; //MOVX();  break; // MOVX Rx, A
        }
    }else{
        switch( m_opcode ){
            case 0x00:          break;   // nop();
            case 0x01: AJMP();  break;
            case 0x02: LJMP();  break;
            case 0x03: RR();    break;
            case 0x04: ACC++;   break;   // Accumulator
            case 0x05:                   // Direct
            case 0x06:
            case 0x07: INC();   break;   // @Indirect Rx

            case 0x10: JBC();   break;
            case 0x11: ACALL(); break;
            case 0x12: LCALL(); break;
            case 0x13: RRC();   break;
            case 0x14: ACC--;   break;   // Accumulator
            case 0x15:                   // Direct
            case 0x16:
            case 0x17: DEC();   break;   // @Indirect Rx

            case 0x20: JB();    break;
            case 0x21: AJMP();  break;
            case 0x22: RET();   break;
            case 0x23: RL();    break;
            case 0x24:                   // #Imminent
            case 0x25:                   // Direct
            case 0x26:
            case 0x27: ADD();   break;   // @Indirect

            case 0x30: JNB();   break;
            case 0x31: ACALL(); break;
            case 0x32: RETI();  break;
            case 0x33: RLC();   break;
            case 0x34:                   // #Imminent
            case 0x35:                   // Direct
            case 0x36:
            case 0x37: ADDC();  break;   // @Indirect

            case 0x40: JC();    break;
            case 0x41: AJMP();  break;
            case 0x42:                   // Direct ACC
            case 0x43: ORLm();  break;   // Direct #I8
            case 0x44:                   // ACC #Imminent
            case 0x45:                   // ACC Direct
            case 0x46:
            case 0x47: ORLa();  break;   // @Indirect

            case 0x50: JNC();   break;
            case 0x51: ACALL(); break;
            case 0x52:                   // Direct ACC
            case 0x53: ANLm();  break;   // Direct #I8
            case 0x54:                   // #Imminent
            case 0x55:                   // Direct
            case 0x56:
            case 0x57: ANLa();  break;   // @Indirect

            case 0x60: JZ();    break;
            case 0x61: AJMP();  break;
            case 0x62:                   // Direct ACC
            case 0x63: XRLm();  break;   // Direct #I8
            case 0x64:                   // #Imminent
            case 0x65:                   // Direct
            case 0x66:
            case 0x67: XRLa();  break;   // @Indirect

            case 0x70: JNZ();   break;
            case 0x71: ACALL(); break;
            case 0x72: ORLc();  break;
            case 0x73: JMP();   break;
            case 0x74: MOVa();  break;   // #Imminent
            case 0x75: MOVm();  break;
            case 0x76:
            case 0x77: MOVm();  break;   // @Indirect

            case 0x80: SJMP();  break;
            case 0x81: AJMP();  break;
            case 0x82: ANLc();  break;
            case 0x83: MOVCp(); break;
            case 0x84: DIVab(); break;
            case 0x85: MOVm();  break;
            case 0x86:
            case 0x87: MOVm();  break;

            case 0x90: MOVd();  break;
            case 0x91: ACALL(); break;
            case 0x92: MOVbc(); break;
            case 0x93: MOVCd(); break;
            case 0x94:                   // #Imminent
            case 0x95:                   // Direct
            case 0x96:
            case 0x97: SUBB();  break;   // @Indirect

            case 0xa0: ORLc();  break;
            case 0xa1: AJMP();  break;
            case 0xa2: MOVc();  break;
            case 0xa3: INCd();  break;
            case 0xa4: MULab(); break;
            case 0xa5: m_PC++;  break;   // unused
            case 0xa6:
            case 0xa7: MOVm();  break;   // @Indirect

            case 0xb0: ANLc();  break;
            case 0xb1: ACALL(); break;
            case 0xb2: CPLb();  break;
            case 0xb3: CPLc();  break;
            case 0xb4:                   // #Imminent
            case 0xb5:                   // Direct
            case 0xb6:
            case 0xb7: CJNE();  break;   // @Indirect

            case 0xc0: PUSH();  break;
            case 0xc1: AJMP();  break;
            case 0xc2: CLRb();  break;
            case 0xc3: CLRc();  break;
            case 0xc4: SWAPa(); break;
            case 0xc5:                   // Direct
            case 0xc6:
            case 0xc7: XCH();   break;   // @Indirect

            case 0xd0: POP();   break;
            case 0xd1: ACALL(); break;
            case 0xd2: SETBb(); break;
            case 0xd3: SETBc(); break;
            case 0xd4: DAa();   break;
            case 0xd5: DJNZ();  break;
            case 0xd6:
            case 0xd7: XCHD();  break;

            case 0xe0: movx_a_indir_dptr(); break;
            case 0xe1: AJMP(); break;
            case 0xe2:
            case 0xe3: movx_a_indir_rx(); break;
            case 0xe4: CLRa(); break;
            case 0xe5:                  // Direct /// Avoid Acc to Acc
            case 0xe6:
            case 0xe7: MOVa(); break;   // @Indirect Rx

            case 0xf0: movx_indir_dptr_a(); break;
            case 0xf1: ACALL(); break;
            case 0xf2:
            case 0xf3: movx_indir_rx_a(); break;
            case 0xf4: CPLa();  break;
            case 0xf5: //MOVx(); break;
            case 0xf6:
            case 0xf7: break; /// m_op0 = ACC; addrInd(); MOVX(); break;  // @Indirect Rx
        }
    }
    bool parity = false;
    uint8_t acu = *m_acc;
    for( int i=0; i<8; ++i )
    {
        parity ^= acu & 1;
        acu >>= 1;
    }
    write_S_Bit( 0, parity );
}
