
// PinModes: undef_mode=0, input, openCo, output, source
var addrBus = -1;
var dataBus = -1;

var pra = 0;


function init()
{
    print("6520 init ");
    
    addrBus = this.getPort("PORTR"); // Addr Bus = 0
    print("addrBus = " + addrBus );
    
    dataBus = this.getPort("PORTD"); // Data Bus = 1
    print("dataBus = " + dataBus );
}

function reset()
{ 
    print("resetting 6520 "); 
    
    this.setPortMode( addrBus, 1 ); // Input
    this.setPortMode( dataBus, 3 ); // Input

}

function extClock( clkState )
{
    print("6520 runClock " + clkState );
    
    this.setPortState( dataBus, clkState );
}
