
bool debug = false;

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
bool m_CS;
bool m_read;
uint8 m_addr;
uint8 m_data;

int getCpuReg( string reg )
{
    //print( "reg = "+reg );
    
    if( reg == "Timer" )  return m_timerCount;
    if( reg == "Presc" )  return m_presc;
    if( reg == "Address") return m_addr;
    if( reg == "Data" )   return m_data;
    return 0;
}

string getStrReg( string reg )
{
    //print( "reg = "+reg ); 
    string value = ""; 
    if( reg == "CS" ) value = m_CS   ? "true" : "false"; 
    if( reg == "Op" ) value = m_read ? "Read" : "Write";
    return value;
}

void command( string c )
{
    //print( "Command: " +c );
    component.toConsole("Ok.\n");
}

void setup() // Executed when Component is created
{
    irqPin.setPinMode( openCo );
    
    component.addCpuReg("Timer","uint8");
    component.addCpuVar("Presc","uint16");
    component.addCpuVar("CS","string");
    component.addCpuVar("Op","string");
    component.addCpuVar("Address","uint16");
    component.addCpuVar("Data","uint8");
    
    print("6532 setup() OK"); 
}

void reset() // Executed at Simulation start
{
    print("6532 reset()"); 
    
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
    //print("6532 runClock " + clkState );
     
    if( m_nextClock != clkState ) return;
    
    m_CS = csPort.getInpState() == 1;  /// Chip Selected: CS1 & !CS2

    
    if( m_nextClock ) risingEdge();
    else              fallingEdge();

    m_nextClock = !m_nextClock;
}

void risingEdge()
{
    //print("6532 Rising Edge "); 
    
    m_addr = addrPort.getInpState();
    m_read = rwPin.getInpState();

    if( m_CS && m_read )                           // MCU is reading
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
    if( !m_CS || m_read )     // Read operation finished
    {
        dataPort.setPinMode( input ); // Release Data Bus
    }
    else             // MCU is writing
    {
        m_data = dataPort.getInpState(); // Read Data Port
        writeREG( m_data );
    }
}

void runEvent()
{
    if( m_read ) // MCU is reading
    {
        m_data = readREG();
        dataPort.setOutState( m_data );       // Set Data Port
    }
}

int readREG()  // MCU is reading
{
    uint8 data = 0;

    if( m_addr & 1<<7 == 0 )              // Read RAM
    {
        if( debug ) print("5532 read RAM "+m_addr+" "+data ); 
        data = component.readRAM( m_addr );
    }
    else                                  // Read Register
    {
        if( m_addr & 1<<2 == 0 )          // PORT Registers
        {
            if( debug ) print("5532 read PORT "+(m_addr & 3) ); 
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
                if( debug ) print("5532 read TIMER "+m_addr+" "+data ); 
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
                if( debug ) print("5532 read FLAGS "+m_addr+" "+data ); 
                data = ifr;               // send IFR before clearing EXTIF flag /// ????
                ifr &= ~EXTIF;            // Clear A7 Interrupt flag
            }
            if( IFR != ifr ){             // Write IFR to RAM
                IFR == ifr;
                component.writeRAM( 0x85, IFR ); // If Interrupt enabled Simulide will remove it from pending
            }
        }
    }
    return data;
}

void writeREG( uint data ) // MCU is writing
{
    //uint8 addr = m_addr;
    
    if( m_addr & 1<<7 == 0 )              // Write RAM
    {
        if( debug ) print("6532 write RAM "+m_addr+" "+data );
        component.writeRAM( m_addr, data );
    }
    else                                  // Write Register
    {
        if( m_addr & 1<<2 == 0 )              // Write PORT Registers
        {
            if( debug ) print("6532 write PORT "+(m_addr & 3)+" "+data );
            uint8 addr;
            switch( m_addr & 3 ){
                case 0: DRA  = data; addr = 0x80; break; // 0x80
                case 1: DDRA = data; addr = 0x81; break; // 0x81
                case 2: DRB  = data; addr = 0x82; break; // 0x82
                case 3: DDRB = data; addr = 0x83; break; // 0x83
            }
            component.writeRAM( addr, data );
        }
        else{                                // Timer & Edge Detect
            uint8 ier = IER;
            
            if( m_addr & 1<<4 == 0 )          // Write Edge Detect control
            {
                if( debug ) print("5532 write EDGE "+m_addr+" "+data );
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
                if( debug ) print("6532 write TIMER "+m_addr+" "+data );
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

