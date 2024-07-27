// CCP1 and CCP2

#define ccpOFF 0b00000000 // Capture/Compare/PWM off (resets ECCP module)
#define capFal 0b00000100 // Capture mode, every falling edge
#define capRis 0b00000101 // Capture mode, every rising edge
#define cap04R 0b00000110 // Capture mode, every 4th rising edge
#define cap16R 0b00000111 // Capture mode, every 16th rising edge
#define comSET 0b00001000 // Compare mode, set output on match (CCP1IF bit is set)
#define comCLR 0b00001001 // Compare mode, clear output on match (CCP1IF bit is set)
#define comInt 0b00001010 // Compare mode, generate software interrupt on match (CCP1IF bit is set, CCP1 pin is unaffected)
#define comSpe 0b00001011 // Compare mode, trigger special event (CCP1IF bit is set; CCP1 resets TMR1 or TMR2

// CCP1 = enhanced:
#define comTog 0b00000010 // Compare mode, toggle output on match (CCP1IF bit is set)
#define pwm1_0 0b00001100 // PWM mode; P1A,C active-high; P1B,D active-high
#define pwm1_1 0b00001101 // PWM mode; P1A,C active-high; P1B,D active-low
#define pwm1_2 0b00001110 // PWM mode; P1A,C active-low;  P1B,D active-high
#define pwm1_3 0b00001111 // PWM mode; P1A,C active-low;  P1B,D active-low

// CCP2:
#define pwm2_0 0b00001100 // PWM mode; 