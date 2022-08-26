// PinModes: undef_mode=0, input, openCo, output, source

var InputPin  = -1;
var outputPin = -1;
var clockPin  = -1;

var PC = 0;
var cycles = 0;
var amply = 0;

function setup()
{
    print("Signal setup ");
    
    InputPin  = this.getPin("In");
    outputPin = this.getPin("Out");
    clockPin  = this.getPin("Clk");
}

function reset()
{ 
    print("resetting Signal"); 
    
    this.setPinMode( InputPin, 1 );     // Input
    this.setPinMode( clockPin, 1 );     // Intput
    this.setPinMode( outputPin, 3 );    // Output
    this.setPinVoltage( outputPin, 0 );
    
    PC = 0;
    cycles = 1;
}

function extClock( clkState )
{
    if( clkState == 0 ) return;
    
    if( cycles == 0 )
    {
        cycles = this.readPGM( PC++ );
        
        if( cycles == 0 ) PC = 0;
        else              amply = this.readPGM( PC++ );
    }
    else cycles--;
    
    var input = this.getPinVoltage( InputPin );
    var output = input*amply/255;
    
    this.setPinVoltage( outputPin, output );
}
