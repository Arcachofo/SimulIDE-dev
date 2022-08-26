
// PinModes: undef_mode=0, input, openCo, output, source

var InputPort = -1;
var outputPin = -1;

function setup()
{
    print("DAC init ");
    
    InputPort = this.getPort("PORTA");
    outputPin = this.getPin("Out");
}

function reset()
{ 
    print("resetting DAC"); 
    
    this.setPortMode( InputPort, 1 ); // Input
    this.setPinMode( outputPin, 3 );  // Output
    this.setPinVoltage( outputPin, 0 );
    
    this.portVoltChanged( InputPort, 1 ); // Register for Input Port voltage changes
}

function voltChanged()
{
    var input = this.getPortState( InputPort );
    print("DAC input changed " + input );
    
    this.setPinVoltage( outputPin, input/51 );
}
