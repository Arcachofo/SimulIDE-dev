/*========================================================================== 
// Author : Handson Technology 
// Project :Arduino Uno with KY040 RotaryEncoderModule
// Description : Reading rotary encoder Value with Arduino Uno 
// Source-Code : KY040_Rotary.ino 
// Module: KY040
//========================================================================== */
int pinA = 3;// Connected to CLK on KY-040
int pinB = 4;// Connected to DT on KY-040
int encoderPosCount = 0;
int pinALast;
int aVal;
boolean bCW;

void setup()
{
    pinMode( pinA, INPUT );
    pinMode( pinB, INPUT );/* Read Pin AWhatever state it's in will reflect the last position   */
    pinALast = digitalRead( pinA );
    Serial.begin (9600);
}

void loop()
{
    aVal = digitalRead( pinA );
    if( aVal != pinALast ) // Means the knob is rotating
    {                   // if the knob is rotating, we need to determine direction
                        // We do that by reading pin B.
        if( digitalRead(pinB)!=aVal ) // Means pin A Changed first -We're RotatingClockwise.
        {
            encoderPosCount ++;
            bCW =true;
        }
        else                        // Otherwise B changedfirst and we're moving CCW
        {
            bCW =false;
            encoderPosCount--;
        }
        Serial.print("Rotated: ");
        if( bCW ) Serial.println("clockwise");
        else      Serial.println("counterclockwise");
        
        Serial.print("Encoder Position: ");
        Serial.println( encoderPosCount );
    }
    pinALast =aVal;
}