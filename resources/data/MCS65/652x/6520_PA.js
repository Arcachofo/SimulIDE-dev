
var pra = 0;

function reset()
{ 
    print("resetting PORTA from Script "); 
}

function configureA( newCRA )
{
    print("PORTA configureA newCRA = " + newCRA ); 
    
    pra = newCRA & (1<<2);
    print("pra = " + pra ); 
}
