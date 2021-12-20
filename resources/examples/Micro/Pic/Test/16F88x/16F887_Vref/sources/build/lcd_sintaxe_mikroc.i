# 1 "/home/user/Escritorio/testVref_887/sources_orig/lcd_sintaxe_mikroc.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "/opt/microchip/mplabx/v5.50/packs/Microchip/PIC16Fxxx_DFP/1.2.33/xc8/pic/include/language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "/home/user/Escritorio/testVref_887/sources_orig/lcd_sintaxe_mikroc.c" 2
# 10 "/home/user/Escritorio/testVref_887/sources_orig/lcd_sintaxe_mikroc.c"
#pragma warning disable 520


# 1 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 1 3
# 13 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef signed char int8_t;






typedef signed int int16_t;







typedef __int24 int24_t;







typedef signed long int int32_t;
# 52 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef unsigned char uint8_t;





typedef unsigned int uint16_t;






typedef __uint24 uint24_t;






typedef unsigned long int uint32_t;
# 88 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef signed char int_least8_t;







typedef signed int int_least16_t;
# 109 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef __int24 int_least24_t;
# 118 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef signed long int int_least32_t;
# 136 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef unsigned char uint_least8_t;






typedef unsigned int uint_least16_t;
# 154 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef __uint24 uint_least24_t;







typedef unsigned long int uint_least32_t;
# 181 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef signed char int_fast8_t;






typedef signed int int_fast16_t;
# 200 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef __int24 int_fast24_t;







typedef signed long int int_fast32_t;
# 224 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef unsigned char uint_fast8_t;





typedef unsigned int uint_fast16_t;
# 240 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef __uint24 uint_fast24_t;






typedef unsigned long int uint_fast32_t;
# 268 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef int32_t intmax_t;
# 282 "/opt/microchip/xc8/v2.32/pic/include/c90/stdint.h" 3
typedef uint32_t uintmax_t;






typedef int16_t intptr_t;




typedef uint16_t uintptr_t;
# 13 "/home/user/Escritorio/testVref_887/sources_orig/lcd_sintaxe_mikroc.c" 2
# 35 "/home/user/Escritorio/testVref_887/sources_orig/lcd_sintaxe_mikroc.c"
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
    if ((Command == 0x01) || (Command == 0x02))
        __delay_ms(2);
}

void Lcd_Goto(uint8_t col, uint8_t row) {
    switch (row) {
    case 2:
        Lcd_Cmd(0xC0 + col - 1);
        break;
    case 3:
        Lcd_Cmd(0x94 + col - 1);
        break;
    case 4:
        Lcd_Cmd(0xD4 + col - 1);
        break;
    default:
        Lcd_Cmd(0x80 + col - 1);
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
    Lcd_Cmd(0x02);
    __delay_ms(5);
    Lcd_Cmd(0x20 | (2 << 2));
    __delay_ms(50);
    Lcd_Cmd(0x0C);
    __delay_ms(50);
    Lcd_Cmd(0x01);
    __delay_ms(50);
    Lcd_Cmd(0x04 | 0x02);
    __delay_ms(50);
}
