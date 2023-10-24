
IoPin@  sdaPin = component.getPin("SDA");
IoPin@  sclPin = component.getPin("SCL");

IoPort@ addrPort = component.getPort("PORTA");

uint m_ctrlCode = 80; // 0x50  01010000
uint m_address = 80; // 0x50  01010000
uint m_steps = 256;
double m_resistance = 100000;

enum pinMode_t{
    undef_mode=0,
    input,
    openCo,
    output,
    source
};

enum twiMode_t{
    TWI_OFF=0,
    TWI_MASTER,
    TWI_SLAVE
};

void setup()
{
    twi.setAddress( m_address );
    print("twi setup() OK"); 
}

void reset()
{
    sdaPin.setPinMode( openCo );
    sclPin.setPinMode( openCo );
    
    addrPort.setPinMode( input );
    addrPort.changeCallBack( element, true );
    
    twi.setMode( TWI_SLAVE );
}

void voltChanged() // Called at addrPort changed
{
    m_address = m_ctrlCode | addrPort.getInpState();
    twi.setAddress( m_address );
    print("twi Address changed "+m_address ); 
}

void byteReceived( uint d )
{
    double data = d*1000/(m_steps-1);
    component.setLinkedValue( 0, data, 0 );
    //print("twi byte received "+d+" data "+data ); 
}

void setControl_Code( uint ctrlCode )
{
    m_ctrlCode = ctrlCode;
}

uint getControl_Code()
{
    return m_ctrlCode;
}

void setSteps( uint steps )
{
    if( steps > 256 ) steps = 256;
    m_steps = steps;
}

uint getSteps()
{
    return m_steps;
}

void setResistance( double res )
{
    //print("twi setResistance "+res ); 
    if( res <= 0 ) return;
    m_resistance = res;
    component.setLinkedValue( 0, res, 1 );
}

double getResistance()
{
    return m_resistance;
}