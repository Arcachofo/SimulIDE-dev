
McuPort@ PortA   = component.getMcuPort("PORTA");
McuPort@ PortB   = component.getMcuPort("PORTB");
IoPort@ csPort   = component.getPort("PORTC");
IoPort@ dataPort = component.getPort("PORTD");
IoPort@ addrPort = component.getPort("PORTR");

IoPin@  irqPin = component.getPin("IRQ");
IoPin@  rwPin  = component.getPin("RW");
McuPin@ a7Pin  = component.getMcuPin("PORTA7");

enum pinModes{
    undef_mode=0, 
    input,
    openCo,
    output,
    source
}

enum extIntTrig{
    pinLow=0,
    pinChange,
    pinFalling,
    pinRising,
    pinDisabled
};

enum intBits{
    EDC0  = 1,
    EXTIE = 1<<1,
    T0IE  = 1<<3,
    EXTIF = 1<<6,
    T0IF  = 1<<7
}

const uint m_rDelay = 10000; // 10 ns

uint8 DRB;
uint8 DRA;
uint8 DDRB;
uint8 DDRA;

uint8 IFR;
uint8 IER;

uint8 m_edc0;
uint m_step;
uint m_presc;
uint m_prescCount;
uint8 m_timerCount;

bool m_nextClock;  // Clock State

bool m_read;
uint8 m_addr;

void setup() // Executed when Component is created
{
    irqPin.setPinMode( openCo );
    
    print("6532 setup() OK"); 
}

void reset() // Executed at Simulation start
{
    print("6532 reset()"); 
    
    irqPin.setOutState( true );
    dataPort.setPinMode( input );
    
    m_nextClock = true; // Wait for first rising edge
    
    DRB  = 0;
    DRA  = 0;
    DDRB = 0;
    DDRA = 0;
    
    IFR = 0;
    IER = 0;
    
    m_edc0 = 0;
    m_presc = 1;
    m_step  = 0;
    m_prescCount = 0;
    m_timerCount = 255;
}

void extClock( bool clkState )  // Function called al clockPin change
{
    print("6532 runClock " + clkState );
     
    if( m_nextClock != clkState ) return;
    
    if( csPort.getInpState() == 1 ) /// Chip Selected: CS1 & !CS2
    {
        if( m_nextClock ) risingEdge();
        else              fallingEdge();
    }
    m_nextClock = !m_nextClock;
}

void risingEdge()
{
    print("6532 Rising Edge "); 
    
    m_addr = addrPort.getInpState();
    m_read = rwPin.getInpState();
    
    if( m_read )                           // MCU is reading
    {
        dataPort.setPinMode( output );
        component.addEvent( m_rDelay );    // Set Data Port after a few ns
    }else{
        dataPort.setPinMode( input );      // We will read data at Falling edge
    }
    
    if( m_step == 0 ) return;

    if( ++m_prescCount == m_step )         // Timer count
    {
        m_prescCount = 0;
        
        if( m_step == 1 )                  // After Interrupt
        {
            if( m_timerCount == 1 ) { m_step = 0; return; }
        }
        else if( m_timerCount == 0 )        // Timer overflow
        {
            m_step = 1;                     // Run at System clock (after Interrupt)
            IFR |= T0IF;                    // Set Timer Interrupt flag
            component.writeRAM( 0x85, IFR );// If Interrupt is enabled it will trigger
        }
        m_timerCount--;
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

int readREG()  // MCU is reading
{
    uint8 data = 0;

    if( m_addr & 1<<7 == 0 )              // Read RAM
    {
        data = component.readRAM( m_addr );
    }
    else                                  // Read Register
    {
        if( m_addr & 1<<2 == 0 )          // PORT Registers
        {
            switch( m_addr & 3 ){
                case 0: data = PortA.getInpState(); break; // 0x80
                case 1: data = DDRA;                break; // 0x81
                case 2: data = PortB.getInpState(); break; // 0x82
                case 3: data = DDRB;                break; // 0x83
            }
        }
        else{                            // Timer & Interrupt
            uint8 ifr = IFR;
            
            if( m_addr & 1 == 0 )         // Read Timer
            {
                uint8 ier = IER;
                uint8 toie = m_addr & T0IE;
                ier &= ~T0IE;             // Enable/Disable Timer Interrupt
                ier |= toie;
                
                if( IER != ier ){
                    IER == ier;
                    component.writeRAM( 0x86, IER ); // Enable/Disable Timer Interrupt
                }
                ifr &= ~T0IF;            // Clear Timer Interrupt flag
                m_step = 0;              // Stop Timer
                
                data = m_timerCount;
            }
            else{                         // Read Interrupt flags
                data = ifr;               // send IFR before clearing EXTIF flag /// ????
                ifr &= ~EXTIF;            // Clear A7 Interrupt flag
            }
            if( IFR != ifr ){             // Write IFR to RAM
                IFR == ifr;
                component.writeRAM( 0x85, IFR ); // If Interrupt enabled Simulide will remove it from pending
            }
        }
    }
    print("6520 readREG "+m_addr+" "+data ); 
    return data;
}

void writeREG( uint data ) // MCU is writing
{
    print("6520 writeREG "+m_addr+" "+data );
    
    uint8 addr = m_addr;
    
    if( m_addr & 1<<7 == 0 )              // Write RAM
    {
        component.writeRAM( m_addr, data );
    }
    else                                  // Write Register
    {
        if( m_addr & 1<<2 == 0 )              // Write PORT Registers
        {
            switch( m_addr & 3 ){
                case 0: data = PortA.getInpState(); break; // 0x80
                case 1: data = DDRA;                break; // 0x81
                case 2: data = PortB.getInpState(); break; // 0x82
                case 3: data = DDRB;                break; // 0x83
            }
        }
        else{                                // Timer & Edge Detect
            uint8 ier = IER;
            
            if( m_addr & 1<<4 == 0 )          // Write Edge Detect control
            {
                uint8 extie = m_addr & EXTIE;
                ier &= ~EXTIE; ier |= extie;  // Enable/Disable A7 Interrupt
                
                uint8 edc0 = m_addr & EDC0;   // Edge mode
                if( m_edc0 != edc0 ){
                    m_edc0 = edc0;
                    ier &= ~EDC0; ier |= edc0;
                    
                    uint edge = edc0 != 0 ? pinRising : pinFalling;
                    
                    a7Pin.setExtInt( edge );  // Set Edge mode
                }
            }
            else{                             // Write Timer
                uint8 toie = m_addr & T0IE; 
                ier &= ~T0IE; ier |= toie;    // Enable/Disable Timer Interrupt
                
                uint8 ifr = IFR;
                ifr &= ~T0IF;                 // Clear Timer Interrupt flag
                m_step = m_presc;             // Run Timer at prescaler speed
                
                if( IFR != ifr ){             // Write IFR to RAM
                    IFR == ifr;
                    component.writeRAM( 0x85, IFR );// If Interrupt enabled Simulide will remove it from pending
                }
                switch( m_addr & 3 ){          // Set prescaler
                    case 0: m_presc = 1;
                    case 1: m_presc = 8;
                    case 2: m_presc = 64;
                    case 3: m_presc = 1024;
                }
            }
            if( IER != ier ){                   // Write IER to RAM
                IER == ier;
                component.writeRAM( 0x86, IER );// Enable/Disable Interrupts
            }
        }
    }
}

int getCpuReg( string reg )
{
    //print( "reg = "+reg );
    if( reg == "DRB"  ) return DRB;
    if( reg == "DRA"  ) return DRA;
    if( reg == "DDRB" ) return DDRB;
    if( reg == "DDRA" ) return DDRA; 
    if( reg == "IFR"  ) return IFR;
    if( reg == "IER"  ) return IER;
    
    if( reg == "Timer"  ) return m_timerCount;
    return 0;
}
