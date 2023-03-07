
McuPort@ PortA   = component.getMcuPort("PORTA");
McuPort@ PortB   = component.getMcuPort("PORTB");
IoPort@ csPort   = component.getPort("PORTC");
IoPort@ dataPort = component.getPort("PORTD");
IoPort@ addrPort = component.getPort("PORTR");

McuPin@ ca1Pin  = component.getMcuPin("CA1");
McuPin@ ca2Pin  = component.getMcuPin("CA2");
McuPin@ cb1Pin  = component.getMcuPin("CB1");
McuPin@ cb2Pin  = component.getMcuPin("CB2");
IoPin@  irqPin  = component.getPin("IRQ");
IoPin@  rwPin   = component.getPin("RW");

enum pinModes{
    undef_mode=0, 
    input,
    openCo,
    output,
    source
}

const uint m_rDelay = 10000; // 10 ns

uint ORB;
uint ORA;
uint DDRB;
uint DDRA;
uint T1CL;   //Timer 1 counter low
uint T1CH;   //Timer 1 counter high
uint T1LL;   // Timer 1 Latch Low
uint T1LH;   // Timer 1 Latch High
uint T2CL;
uint T2CH;
uint SR;
uint ACR;    // AuxiliaryControl Register
uint PCR;    // Peripheral Control Register
uint IRB;
uint IRA;

enum acrBits{
    PA   = 1,
    PB   = 2,
    SRC0 = 4,
    SRC1 = 8,
    SRC2 = 16,
    T2C  = 32,
    T1C0 = 64,
    T1C1 = 128
}
enum pcrBits{
    CA1c = 1,
    CA20 = 2,
    CA21 = 4,
    CA22 = 8,
    CB1c = 16,
    CB20 = 32,
    CB21 = 64,
    CB22 = 128
}

uint m_addr;

bool m_nextClock;  // Clock State
bool m_read;

bool m_latchA;
bool m_latchB;
bool m_pulseCA2;
bool m_pulseCB2;

int m_CA2ctrl;
int m_CB2ctrl;

bool m_T2ctrl;
bool m_T1cont;
bool m_T1pb7;

void setup() // Executed at setScript()
{
    irqPin.setPinMode( openCo );
    
    print("6522 setup() OK"); 
}

void reset() // Executed at Simulation start
{ 
    print("6522 reset()");
    
    irqPin.setOutState( true );
    dataPort.setPinMode( input );
    
    ca2Pin.setPortState( true );
    
    m_nextClock = true; // Wait for first rising edge
    
    m_pulseCA2 = false;
    m_pulseCB2 = false;
    
    m_latchA = false;
    m_latchB = false;
    
    m_CA2ctrl = -1;
    m_CB2ctrl = -1;
    
    ORB  = 0;
    ORA  = 0;
    DDRB = 0;
    DDRA = 0;
    T1CL = 0;
    T1CH = 0;
    T1LL = 0;
    T1LH = 0;
    T2CL = 0;
    T2CH = 0;
    SR   = 0;
    ACR  = 0;
    PCR  = 0;
}

void extClock( bool clkState )  // Function called al clockPin change
{
    if( m_nextClock != clkState ) return;
    
    if( csPort.getInpState() == 1 ) // Chip Selected
    {
        if( m_nextClock ) risingEdge();
        else              fallingEdge();
    }
    m_nextClock = !m_nextClock;
}

void risingEdge()
{
    m_addr = addrPort.getInpState();
    m_read = rwPin.getInpState();
    
    if( m_read )     // MCU is reading
    {
        dataPort.setPinMode( output );
        component.addEvent( m_rDelay ); // Set Data Port after a few ns
    }else{
        dataPort.setPinMode( input );   // We will read data at Falling edge
    }
}

void fallingEdge()
{
    if( !m_read ) // MCU is writing
    {
        uint data = dataPort.getInpState(); // Read Data Port
        writeREG( data );
    }
}

void runEvent()
{
    if( m_read ) // MCU is reading
    {
        uint data = readREG();
        dataPort.setOutState( data );       // Set Data Port
    }
}

uint readREG()
{
    uint data = 0;
    switch( m_addr ){
        case 0: {
            data = ORB & DDRB;                  // Out Pin: bit from Register
            if( m_latchB ) data |= IRB & ~DDRB; // latched at CB1 change
            else           data |= PortB.getInpState() & ~DDRB;// In  Pin: bit from Pin
            IRA = data;
        }break;
        case 1:{
            if( m_latchA ) data = IRA;          // Latched at CA1 change
            else           data = PortA.getInpState();
            IRA = data;
        }break;
        case 2: data = DDRB; break;
        case 3: data = DDRA; break;
        case 4:{
            data = T1CL; break;
            // T1 interrupt flag IFR6 is reset.
            } break;
        case 5: data = T1CH; break;
        case 6: data = T1LL; break;
        case 7: data = T1LH; break;
        case 8: break; 
        case 9: break;
        case 10: data = SR; break;
        case 11: data = ACR; break;
        case 12: data = PCR; break;
    }
    return data;
}

void writeREG( uint data )
{
    switch( m_addr ){
        case 0: ORB  = data; break;
        case 1: ORA  = data; break;
        case 2: DDRB = data; break;
        case 3: DDRA = data; break;
        case 4: T1LL = data; break; // T1CL Load data into latch Low
        case 5:{                    // T1CH
            T1LH = data;            // load data into latch High
            T1CH = T1LH;
            T1CL = T1LL;
            // initiates countdown
            // T1 interrupt flag IFR6 is reset.
        } break;
        case 6: T1LL = data; break; 
        case 7:{ 
            T1LH = data; break; // T1CH load data into latch High
            // T1 interrupt flag IFR6 is reset.
        } break;
        case 8: break; 
        case 9: break; 
        case 10: SR = data; break; 
        case 11:{ // ACR
            ACR = data;
            writeACR();
        } break;
        case 12:{ // PCR
            PCR = data;
            writePCR();
        } break;
    }
    component.writeRAM( m_addr, data ); // Mcu Mon, Dir changed, Int en/dis, etc.
}

void writeACR()
{
    m_latchA = (ACR & PA) > 0;
    m_latchB = (ACR & PB) > 0;
    
    m_T2ctrl = (ACR & T2C) > 0;
    
    // SR
    
    m_T1cont = (ACR & T1C0) > 0;
    m_T1pb7  = (ACR & T1C1) > 0;
}

void writePCR()
{
    if( (PCR & CA1c) > 0 ) ca1Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
    else                   ca1Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
    
    if( (PCR & CA22) > 0 ) // CA2 Control
    {
        m_CA2ctrl = -1;
        if( (PCR & CA21) > 0 ) ca2Pin.setPortState( (PCR & CA20) > 0 ); // Manual Output
        else                   m_CA2ctrl = PCR & CA20;// Handshake/Pulse Output
    }else{
        if( (PCR & CA21) > 0 ) ca2Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
        else                   ca2Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
        /// if( (PCR & CA20) > 0 ) ;//Independent interrupt
    }
    
    if( (PCR & CB1c) > 0 ) cb1Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
    else                   cb1Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
    
    if( (PCR & CA22) > 0 ) // CB2 Control
    {
        m_CB2ctrl = -1;
        if( (PCR & CB21) > 0 ) cb2Pin.setPortState( (PCR & CB20) > 0 ); // Manual Output
        else                   m_CB2ctrl = PCR & CB20;// Handshake/Pulse Output
    }else{
        if( (PCR & CB21) > 0 ) cb2Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
        else                   cb2Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
        /// if( (PCR & CB20) > 0 ) ;//Independent interrupt
    }
}

void INTERRUPT( uint vector )  // Called to execute an interrupt
{
    print("6522 INTERRUPT() "+vector); 
}

