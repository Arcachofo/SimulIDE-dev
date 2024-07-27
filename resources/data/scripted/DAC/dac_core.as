
// PinModes: undef_mode=0, input, openCo, output, source

IoPort@ InputPort = component.getPort("PORTA");
IoPin@  outputPin = component.getPin("Out");

void setup()
{
    print("DAC init ");
}

void reset()
{ 
    print("resetting DAC"); 
    
    InputPort.setPinMode( 1 ); // Input
    outputPin.setPinMode(3 );  // Output
    outputPin.setVoltage( 0 );
    
    InputPort.changeCallBack( element, true ); // Register for Input Port voltage changes
}

void voltChanged()
{
    double input = InputPort.getInpState();
    print("DAC input changed " + input );
    
    outputPin.setVoltage( input/51 );
}
