/*******************************************************************************
 * LCD_Lib.c                                                                   *
 * MPLAB XC8 compiler LCD driver for LCDs with HD44780 compliant controllers.  *
 * https://simple-circuit.com/                                                 *
 *                                                                             *
 * Sintaxe adaptada para mikroC por chicodorea@academico.ufs.br (01/nov/21)    *
 ******************************************************************************/

 # pragma warning disable 520

 # include <stdint.h>

 # define _LCD_FIRST_ROW 0x80
 # define _LCD_SECOND_ROW 0xC0
 # define _LCD_THIRD_ROW 0x94
 # define _LCD_FOURTH_ROW 0xD4
 # define _LCD_CLEAR 0x01
 # define _LCD_RETURN_HOME 0x02
 # define _LCD_ENTRY_MODE_SET 0x04
 # define _LCD_CURSOR_OFF 0x0C
 # define _LCD_UNDERLINE_ON 0x0E
 # define _LCD_BLINK_CURSOR_ON 0x0F
 # define _LCD_MOVE_CURSOR_LEFT 0x10
 # define _LCD_MOVE_CURSOR_RIGHT 0x14
 # define _LCD_TURN_ON 0x0C
 # define _LCD_TURN_OFF 0x08
 # define _LCD_SHIFT_LEFT 0x18
 # define _LCD_SHIFT_RIGHT 0x1E

 # ifndef Lcd_Type
 # define Lcd_Type 2 // 0=5x7, 1=5x10, 2=2 lines
 # endif

__bit RS;

void Lcd_Write_Nibble(uint8_t n);
void Lcd_Cmd(uint8_t Command);
void Lcd_Goto(uint8_t col, uint8_t row);
void Lcd_PutC(char Dt_Char);
void Lcd_Print(char * Dt_Str);
void Lcd_Println(const char dado[]);
void Lcd_Chr(uint8_t row, uint8_t col, char Dt_Char);
void Lcd_Chr_CP(char Dt_Char);
void Lcd_Write(uint8_t row, uint8_t col, char * Dt_Str);
void Lcd_Out(uint8_t row, uint8_t col, char * Dt_Str);
void Lcd_Init();

void Lcd_Write_Nibble(uint8_t n) {
    LCD_RS = RS;
    LCD_D4 = n & 0x01;
    LCD_D5 = (n >> 1) & 0x01;
    LCD_D6 = (n >> 2) & 0x01;
    LCD_D7 = (n >> 3) & 0x01;

    // send enable pulse
    LCD_EN = 0;
    __delay_us(1);
    LCD_EN = 1;
    __delay_us(1);
    LCD_EN = 0;
    __delay_us(100);
}

void Lcd_Cmd(uint8_t Command) {
    RS = 0;
    Lcd_Write_Nibble(Command >> 4);
    Lcd_Write_Nibble(Command);
    if ((Command == _LCD_CLEAR) || (Command == _LCD_RETURN_HOME))
        __delay_ms(2);
}

void Lcd_Goto(uint8_t col, uint8_t row) {
    switch (row) {
    case 2:
        Lcd_Cmd(_LCD_SECOND_ROW + col - 1);
        break;
    case 3:
        Lcd_Cmd(_LCD_THIRD_ROW + col - 1);
        break;
    case 4:
        Lcd_Cmd(_LCD_FOURTH_ROW + col - 1);
        break;
    default: // case 1:
        Lcd_Cmd(_LCD_FIRST_ROW + col - 1);
    }
}

void Lcd_PutC(char Dt_Char) {
    RS = 1;
    Lcd_Write_Nibble(Dt_Char >> 4);
    Lcd_Write_Nibble(Dt_Char);
}

void Lcd_Print(char * Dt_Str) {
    uint8_t i = 0;
    RS = 1;
    while (Dt_Str[i] != '\0') {
        Lcd_Write_Nibble(Dt_Str[i] >> 4);
        Lcd_Write_Nibble(Dt_Str[i++]);
    }
}

void Lcd_Println(const char Dt_Str[]) {
    uint8_t i = 0;
    RS = 1;
    while (Dt_Str[i] != '\0') {
        Lcd_Write_Nibble(Dt_Str[i] >> 4);
        Lcd_Write_Nibble(Dt_Str[i++]);
    }
}

void Lcd_Chr(uint8_t row, uint8_t col, char Dt_Char) {
    Lcd_Goto(col, row);
    Lcd_PutC(Dt_Char);
}

void Lcd_Chr_CP(char Dt_Char) {
    Lcd_PutC(Dt_Char);
}

void Lcd_Write(uint8_t row, uint8_t col, char * Dt_Str) {
    Lcd_Goto(col, row);
    Lcd_Print(Dt_Str);
}

void Lcd_Out(uint8_t row, uint8_t col, char * Dt_Str) {
    Lcd_Goto(col, row);
    Lcd_Print(Dt_Str);
}

void Lcd_Init() {
    RS = 0;

    LCD_RS = 0;
    LCD_EN = 0;
    LCD_D4 = 0;
    LCD_D5 = 0;
    LCD_D6 = 0;
    LCD_D7 = 0;
    LCD_RS_DIR = 0;
    LCD_EN_DIR = 0;
    LCD_D4_DIR = 0;
    LCD_D5_DIR = 0;
    LCD_D6_DIR = 0;
    LCD_D7_DIR = 0;

    __delay_ms(40);
    Lcd_Cmd(3);
    __delay_ms(5);
    Lcd_Cmd(3);
    __delay_ms(5);
    Lcd_Cmd(3);
    __delay_ms(5);
    Lcd_Cmd(_LCD_RETURN_HOME);
    __delay_ms(5);
    Lcd_Cmd(0x20 | (Lcd_Type << 2));
    __delay_ms(50);
    Lcd_Cmd(_LCD_TURN_ON);
    __delay_ms(50);
    Lcd_Cmd(_LCD_CLEAR);
    __delay_ms(50);
    Lcd_Cmd(_LCD_ENTRY_MODE_SET | _LCD_RETURN_HOME);
    __delay_ms(50);
}
