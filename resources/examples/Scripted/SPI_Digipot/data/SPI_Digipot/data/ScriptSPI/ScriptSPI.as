
IoPin@ csPin = component.getPin("CS");

bool m_enabled;
uint m_steps = 256;
double m_resistance = 100000;

enum pinMode_t{
    undef_mode=0,
    input,
    openCo,
    output,
    source
};

enum spiMode_t{
    SPI_OFF=0,
    SPI_MASTER,
    SPI_SLAVE
};

void setup()
{
    print("script SPI setup() OK"); 
}

void reset()
{
    m_enabled = true;
    
    csPin.setPinMode( input );
    csPin.changeCallBack( element, true );
    
    spi.setMode( SPI_SLAVE );
}

void voltChanged() // Called at csPin changed
{
    bool enabled = !csPin.getInpState();
    if( m_enabled == enabled ) return;
    m_enabled = enabled;
    
    if( enabled ) spi.setMode( SPI_SLAVE );
    else          spi.setMode( SPI_OFF );
    //print("script SPI "+m_address ); 
}

void byteReceived( uint d )
{
    double data = d*1000/(m_steps-1);
    component.setLinkedValue( 0, data, 0 );
    //print("script SPI byte received "+d+" data "+data ); 
}

void setSteps( uint steps )
{
    if( steps > 256 ) steps = 256; // We are receiving 1 byte
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