
byte byteRead;

void setup() 
{
    Serial.begin(9600);
    
    Serial.write("Serial echo Test\n");
}

void loop() 
{
    if( Serial.available() ) 
    {
        byteRead = Serial.read();
    
        Serial.write(byteRead);
    }
}file:///mnt/software/simulide/simulide_lauchpad/trunk/resources/examples/Micro/Arduino/Keypad/HelloKeypad/HelloKeypad.ino
