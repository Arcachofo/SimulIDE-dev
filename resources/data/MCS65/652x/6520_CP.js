
function reset()
{ 
    //print("resetting CP from Script "); 
}

function configureA( newCRA )
{
    //print("CP configureA newCRA = " + newCRA ); 
    
    // CA1 interrupt edge
    var intTrig = newCRA & (1<<1); //
    if( intTrig > 0 ) intTrig = 3; // Interrrupt at Rising edge
    else              intTrig = 2; // Interrrupt at Falling edge
    this.setExtIntTrig( 0, intTrig );
    
    var cra3 = newCRA & (1<<3);
    var cra4 = newCRA & (1<<4);
    var cra5 = newCRA & (1<<5);
    
    if( cra5 == 0 )// CA2 interrupt edge (Input)
    {
        // Set CA2 as input
        var intTrig = 0;
        if( cra4 > 0 ) intTrig = 3; // Interrrupt at Rising edge
        else           intTrig = 2; // Interrrupt at Falling edge
        this.setExtIntTrig( 1, intTrig );
    }
    else           // CA2 control line (Output)
    {
        // Set CA2 as output
        if( cra4 > 0 ) ; // CA2 = cra4
        else
        {
            if( cra3 > 0 ) ; // LOW with p2 after Read ORA, High with p2 low during deselect
            else           ; // LOW with p2 after Read ORA, High with CRA7 (IRQA1)
        }
    }
}
