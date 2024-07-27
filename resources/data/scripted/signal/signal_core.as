
// PinModes: undef_mode=0, input, openCo, output, source


uint PC = 0;
int cycles = 0;
int ampli = 0;

IoPin@ inputPin  = component.getPin("In");
IoPin@ outputPin = component.getPin("Out");
IoPin@ clockPin  = component.getPin("Clk");

void setup() // Executed at setScript()
{
    print("AS setup() Doing Nothing"); 
}

void reset() // Executed at Simulation start
{ 
    print("AS reset()"); 
    
    inputPin.setPinMode( 1 );     // Input
    clockPin.setPinMode( 1 );     // Intput
    outputPin.setPinMode( 3 );    // Output
    outputPin.setVoltage( 0 );
    
    PC = 0;
    cycles = 1;
}

void extClock( bool clkState )  // Function called 1e6 times per sec.
{
    if( cycles == 0 )
    {
        cycles = component.readPGM( PC++ );
        
        if( cycles == 0 ) PC = 0;
        else              ampli = component.readPGM( PC++ );
    }
    else cycles--;
    
    double input  = inputPin.getVoltage();
    double output = input*ampli/255;
    
    outputPin.setVoltage( output );
}
