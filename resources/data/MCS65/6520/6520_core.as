
McuPort@ PortA   = component.getMcuPort("PORTA");
McuPort@ PortB   = component.getMcuPort("PORTB");
IoPort@ csPort   = component.getPort("PORTC");
IoPort@ dataPort = component.getPort("PORTD");
IoPort@ addrPort = component.getPort("PORTR");

McuPin@ ca1Pin  = component.getMcuPin("CA1");
McuPin@ ca2Pin  = component.getMcuPin("CA2");
McuPin@ cb1Pin  = component.getMcuPin("CB1");
McuPin@ cb2Pin  = component.getMcuPin("CB2");
IoPin@  irqPinA = component.getPin("IRQA");
IoPin@  irqPinB = component.getPin("IRQB");
IoPin@  rwPin   = component.getPin("RW");

enum pinModes{
    undef_mode=0, 
    input,
    openCo,
    output,
    source
}

const uint m_rDelay = 10000; // 10 ns

uint DDRA;
uint CRA;
uint DDRB;
uint CRB;
uint ORA;
uint ORB;

enum crBits{
    Cx10  = 1,
    Cx11  = 2,
    PRx0  = 4,
    Cx20  = 8,
    Cx21  = 16,
    Cx22  = 32,
    IRQx2 = 64,
    IRQx1 = 128
}

uint m_PIA;
uint m_PIB;
int m_CA2ctrl;
int m_CB2ctrl;

bool m_read;
uint m_addr;

bool m_nextClock;  // Clock State
bool m_pulseCA2;
bool m_pulseCB2;

void setup() // Executed when Component is created
{
    csPort.setPinMode( input );
    addrPort.setPinMode( input );
    
    irqPinA.setPinMode( openCo );
    irqPinB.setPinMode( openCo );
    
    print("6520 setup() OK"); 
}

void reset() // Executed at Simulation start
{
    print("6520 reset()"); 
    
    dataPort.setPinMode( input ); 
    
    irqPinA.setOutState( true );
    irqPinB.setOutState( true );
    
    m_nextClock = true; // Wait for first rising edge
    m_pulseCA2 = false;
    m_pulseCB2 = false;
    
    m_CA2ctrl = -1;
    m_CB2ctrl = -1;

    DDRA = 0;
    CRA  = 0;
    DDRB = 0;
    CRB  = 0;
    ORA  = 0;
    ORB  = 0;
}

void extClock( bool clkState )  // Function called al clockPin change
{
    if( m_nextClock != clkState ) return;
    
    if( csPort.getInpState() == 3 ) // Chip Selected
    {
        if( m_nextClock ) risingEdge();
        else              fallingEdge();
    }
    m_nextClock = !m_nextClock;
}

void risingEdge()
{
    //print("6520 Rising Edge "); 
    
    m_addr = addrPort.getInpState();
    m_read = rwPin.getInpState();
    
    if( m_pulseCA2 ){
        m_pulseCA2 = false;
        ca2Pin.setPortState( true );
    }
    if( m_pulseCB2 ){
        m_pulseCB2 = false;
        cb2Pin.setPortState( false );
    }
    
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

int readREG()
{
    uint data = 0;
    switch( m_addr ){
        case 0:{
            if( m_PIA > 0 )                 // Read ORA
            {
                data = PortA.getInpState();
                CRA &= 0b00111111;          // Clear Interrupt flags
                irqPinA.setOutState( true );// Interrupt Pin High
                
                if( m_CA2ctrl != -1 )       // Handshake/Pulse Output
                {
                    ca2Pin.setPortState( false );
                    if( m_CA2ctrl == Cx20 ) m_pulseCA2 = true;
                }
            }
            else data = DDRA;               // Read DDRA
        } break;
        case 1: data = CRA; break;          // Read CRA
        case 2:{
            if( m_PIB > 0 )                 // Read ORB/CRB
            {
                data = ORB & DDRB;                  // Out Pin: bit from Register
                data |= PortB.getInpState() & ~DDRB;// In  Pin: bit from Pin
                CRB &= 0b00111111;          // Clear Interrupt flags
                irqPinB.setOutState( true );// Interrupt Pin High
            }
            else data = DDRB;               // Read DDRB
        } break;
        case 3: data = CRB; break;          // Read CRB
    }
    //print("6520 readREG "+m_addr+" "+data ); 
    return data;
}

void writeREG( uint data )
{
    //print("6520 writeREG "+m_addr+" "+data ); 
    switch( m_addr ){
        case 0:{                            // ORA/DDRA
            m_addr += m_PIA;
            if( m_addr == 0 ) DDRA = data;  // Write DDRA
            else              ORA  = data;  // Write ORA
        } break;
        case 1:{                            // Write CRA 
            CRA = data;
            writeCRA();
        } break;
        case 2:{                            // ORB/DDRB
            m_addr += m_PIB;
            if( m_addr == 2 ) DDRB = data;  // Write DDRB
            else{                           // Write ORB
                ORB  = data;
                if( m_CB2ctrl != -1 )       // Handshake/Pulse Output
                {
                    cb2Pin.setPortState( false );
                    if( m_CB2ctrl == Cx20 ) m_pulseCB2 = true;
                }
            }
        } break;
        case 3:{                            // Write CRB
            CRB = data;
            writeCRB();
        } break;
    }
    component.writeRAM( m_addr, data ); // Mcu Mon, Dir changed, Int en/dis, etc.
}

void writeCRA()
{
    m_PIA = CRA & PRx0; // PRA0

    if( (CRA & Cx11) > 0 ) ca1Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
    else                   ca1Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
    
    int ca22 = CRA & Cx22;
    if( ca22 == 0 ){
        if( (CRA & Cx21) > 0 ) ca2Pin.setExtInt( 3 ); // Interrupt at CA2 Rising edge
        else                   ca2Pin.setExtInt( 2 ); // Interrupt at CA2 Falling edge
        ca2Pin.setDirection( false );                 // CA2 Pin as Input
    }else{
        ca2Pin.setDirection( true );                  // CA2 Pin as Output
        ca2Pin.setExtInt( 5 );                        // Don't trigger interrupt
        m_CA2ctrl = -1;
        if( (CRA & Cx21) > 0 ) ca2Pin.setPortState( (CRA & Cx20) > 0 ); // Manual Output
        else                   m_CA2ctrl = CRA & Cx20;// Handshake/Pulse Output
    }
}

void writeCRB()
{
    m_PIB = CRB & PRx0; // PRB0
    
    if( (CRB & Cx11) > 0 ) cb1Pin.setExtInt( 3 ); // Interrupt at CA1 Rising edge
    else                   cb1Pin.setExtInt( 2 ); // Interrupt at CA1 Falling edge
    
    int ca22 = CRB & Cx22;
    if( ca22 == 0 ){
        if( (CRB & Cx21) > 0 ) cb2Pin.setExtInt( 3 ); // Interrupt at CA2 Rising edge
        else                   cb2Pin.setExtInt( 2 ); // Interrupt at CA2 Falling edge
        cb2Pin.setDirection( false );                 // CB2 Pin as Input
    }else{
        cb2Pin.setDirection( true );                  // CB2 Pin as Output
        cb2Pin.setExtInt( 5 );                        // Don't trigger interrupt
        m_CB2ctrl = -1;
        if( (CRB & Cx21) > 0 ) cb2Pin.setPortState( (CRB & Cx20) > 0 ); // Manual Output
        else                   m_CB2ctrl = CRB & Cx20;// Handshake/Pulse Output
    }
}

void INTERRUPT( uint vector )  // Called to execute an interrupt
{
    //print("6520 INTERRUPT() "+vector);
    
    if( vector == 0 ){                                    // IRQA
        irqPinA.setOutState( false );                     // Lower IRQA Pin
        if( m_CA2ctrl == 0 ) ca2Pin.setPortState( true ); // Handshake
    }else{                                                // IRQB
        irqPinB.setOutState( false );                     // Lower IRQB Pin
        if( m_CB2ctrl == 0 ) cb2Pin.setPortState( true ); // Handshake
    }
}
