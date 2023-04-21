/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "connector.h"
#include "simulator.h"
#include "hd44780_base.h"
#include "utils.h"

Hd44780_Base::Hd44780_Base( QObject* parent, QString type, QString id )
            : Component( parent, type, id )
            , m_fontImg( ":font2.png" )
{
    m_graphical = true;
    
    m_rows = 2;
    m_cols = 16;

    setLabelPos( 70,-82, 0);
    setShowId( true );
}
Hd44780_Base::~Hd44780_Base(){}

void Hd44780_Base::init()
{
    m_lastCircTime = Simulator::self()->circTime();
    m_lastClock = false;
    m_writeDDRAM = true;
    m_blinking = false;
    m_cursPos     = 0;
    m_shiftPos    = 0;
    m_direction   = 1;
    m_shiftDisp   = 0;
    m_dispOn      = 0;
    m_cursorOn    = 0;
    m_cursorBlink = 0;
    m_dataLength  = 8;
    m_lineLength  = 80;
    m_nibble      = 0;
    m_input = 0;
    
    m_DDaddr = 0;
    m_CGaddr = 0;
    
    m_imgWidth  = (m_cols*6-1)*2;
    m_imgHeight = (m_rows*9-1)*2;
    m_area = QRectF( 0, -(m_imgHeight+33), m_imgWidth+20, m_imgHeight+33 );
    setTransformOriginPoint( toGrid( m_area.center() ));

    clearLcd();
}

void Hd44780_Base::writeData( int data )
{
    if( m_writeDDRAM )                                 // Write to DDRAM
    {
        m_DDram[m_DDaddr] = data;
        m_DDaddr += m_direction;
        
        if( m_DDaddr > 79 ) m_DDaddr = 0;
        if( m_DDaddr < 0 )  m_DDaddr = 79;
        
        if( m_shiftDisp )
        {
            m_shiftPos += m_direction;
            int lineEnd = m_lineLength-1;

            if( m_shiftPos>lineEnd ) m_shiftPos = 0;
            if( m_shiftPos<0 )       m_shiftPos = lineEnd;
    }   }
    else{                                             // Write to CGRAM
        m_CGram[m_CGaddr] = data;
        m_CGaddr += 1;
        
        if( m_CGaddr > 63 ) m_CGaddr = 0;
}   }

void Hd44780_Base::proccessCommand( int command )
{
    //qDebug() << "Hd44780_Base::proccessCommand: " << command;
    if( command == 0 )  return;
    if( command & 128){ setDDaddr( command-128 );return; } //1....... Set DDRAM address    Sets the DDRAM address. DDRAM data are sent and received after this setting 37 μs
    if( command & 64) { setCGaddr( command-64 ); return; } //01...... Set CGRAM address    Sets the CGRAM address. CGRAM data are sent and received after this setting 37 μs
    if( command & 32) { functionSet( command );  return; } //001..... Function set         Sets interface data length(DL), nº of display line(N), and character font(F) 37 μs
    if( command & 16) { C_D_Shift( command );    return; } //0001.... Cursor/display shift Sets cursor-move or display-shift(S/C), shift direction(R/L). DDRAM unchanged 37 μs
    if( command & 8 ) { dispControl( command );  return; } //00001... Display on/off       Sets on/off of all display(D), cursor on/off(C), blink of cursor position character(B) 37 μs
    if( command & 4 ) { entryMode( command );    return; } //000001.. Entry mode set       Sets cursor move direction(I/D); Shift the display(S). Performed during data read/write. 37 μs
    if( command & 2 ) { cursorHome();            return; } //0000001. Cursor home          Cursor to home position. Returns display being shifted to original position. DDRAM unchanged. 1.52 ms
    if( command & 1 ) { clearLcd();              return; } //00000001 Clear display        Clears display and returns cursor to the home position (address 0). 1.52 ms
}

void Hd44780_Base::functionSet( int data )
{
    if( data & 16 ) m_dataLength = 8;    // Data Length
    else            m_dataLength = 4;
    
    if( data & 8 ) m_lineLength = 40;    // Display Lines 
    else           m_lineLength = 80;
    
    // Sets the character font.
    //if( data & 4 ) ;
    //else            ;
    //qDebug()<<m_dataLength<<m_lineLength<<(data & 16);
}
  
void Hd44780_Base::C_D_Shift( int data )
{
    int dir = 1;    // Move Right/Left
    if( data & 4 ) dir = -1;

    if( data & 8 )     // Shift Cursor/Display
    {
        m_shiftPos += dir;
        int lineEnd = m_lineLength-1;

        if( m_shiftPos>lineEnd ) m_shiftPos = 0;
        if( m_shiftPos<0 )       m_shiftPos = lineEnd;
    }
    else m_cursPos  += dir;
}

void Hd44780_Base::dispControl( int data )
{
    if( data & 4 ) m_dispOn = 1;                       // Display On/Off
    else           m_dispOn = 0;
    
    if( data & 2 ) m_cursorOn = 1;                      // Cursor On/Off
    else           m_cursorOn = 0;
    
    if( data & 1 ) m_cursorBlink = 1;                    // Cursor Blink
    else           m_cursorBlink = 0;
}

void Hd44780_Base::entryMode( int data )
{
    // Cursor move left/right
    if( data & 2 ) m_direction = 1;
    else           m_direction = -1;
    
    // Shift Display
    if( data & 1 ) m_shiftDisp = 1;
    else           m_shiftDisp = 0;
}

void Hd44780_Base::clearLcd()
{
    clearDDRAM();
    cursorHome();
}

void Hd44780_Base::cursorHome()
{
    m_DDaddr   = 0;
    m_cursPos  = 0;
    m_shiftPos = 0;
}

void Hd44780_Base::setDDaddr( int addr )
{
    if( (m_lineLength==40) & (addr>63) ) addr -= 24;
    m_DDaddr = addr & 0b01111111;
    
    m_writeDDRAM = true;
}

void Hd44780_Base::setCGaddr( int addr )
{
    m_CGaddr = addr & 0b00111111;
    m_writeDDRAM = false;
}

void Hd44780_Base::clearDDRAM()
{
    for(int i=0; i<80; i++) m_DDram[i] = 32;
}

void Hd44780_Base::setCols( int cols )
{
    if( cols > 20 ) cols = 20;
    if( cols < 8 ) cols = 8;
    m_cols = cols;
    init();
}

void Hd44780_Base::setRows( int rows )
{
    if( rows > 4 ) rows = 4;
    if( rows < 1 ) rows = 1;
    m_rows = rows;
    init();
}

void Hd44780_Base::showPins( bool show )
{
    m_pinRS->setVisible( show );
    m_pinRW->setVisible( show );
    m_pinEn->setVisible( show );
    
    for( int i=0; i<8; i++ ) m_dataPin[i]->setVisible( show );
}

void Hd44780_Base::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );

    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area, 2, 2 );
    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( 4, -(29+m_imgHeight), m_imgWidth+12, m_imgHeight+12, 8, 8 );

    if( m_dispOn == 0 ) return;

    uint64_t circTime = Simulator::self()->circTime();
    if( circTime-m_lastCircTime >= 409600*1e6 )  // Blink period = 409.6 ms
    {
        m_lastCircTime = circTime;
        if( m_cursorBlink ) m_blinking = !m_blinking;
        else                m_blinking = false;
    }

    for( int row=0; row<m_rows; row++ )
    {
        for( int col=0; col<m_cols; col++ )
        {
            int mem_pos = 0;
            if( row < 2 ) mem_pos += row*40+col;
            else          mem_pos += (row-2)*40+20+col;

            int lineEnd = 79;
            int lineStart = 0;

            if( m_lineLength == 40 )
            {
                if( mem_pos < 40 ) lineEnd   = 39;
                else               lineStart = 40;
            }
            mem_pos += m_shiftPos;
            if( mem_pos>lineEnd )   mem_pos -= m_lineLength;
            if( mem_pos<lineStart ) mem_pos += m_lineLength;

            int char_num = m_DDram[mem_pos];
            QImage charact( 10, 16, QImage::Format_RGB32 );

            if( mem_pos == m_DDaddr )
            {
                if( m_blinking )     // Draw black instead of character
                {
                    charact.fill(qRgb(0, 0, 0));
                    p->drawImage(10+col*12,-(m_imgHeight+22)+row*18,charact );
                    continue;
                }
                if( m_cursorOn )     // Draw cursor if enabled
                {
                    charact = m_fontImg.copy( 0, 0, 10, 16 );
                    int y = 14;
                    for( int x=9; x>0; x-=2 )
                    {
                        charact.setPixel(x,   y,   qRgb(0, 0, 0));
                        charact.setPixel(x-1, y,   qRgb(0, 0, 0));
                        charact.setPixel(x,   y+1, qRgb(0, 0, 0));
                        charact.setPixel(x-1, y+1, qRgb(0, 0, 0));
                    }
                    p->drawImage(10+col*12,-(m_imgHeight+22)+row*18,charact );
            }   }
            if( char_num < 8 )                        // CGRam Character
            {
                charact = m_fontImg.copy( 0, 0, 10, 16 );
                int addr = char_num*8;

                for( int y=0; y<16; y+=2 )
                {
                    int data = m_CGram[ addr ];
                    addr++;

                    for( int x=9; x>0; x-=2 )
                    {
                        if( data & 1 ) {
                            charact.setPixel(x,   y,   qRgb(0, 0, 0));
                            charact.setPixel(x-1, y,   qRgb(0, 0, 0));
                            charact.setPixel(x,   y+1, qRgb(0, 0, 0));
                            charact.setPixel(x-1, y+1, qRgb(0, 0, 0));
                        }
                        data = data>>1;
            }   }   }
            else charact = m_fontImg.copy( char_num*10, 0, 10, 16 );

            p->drawImage( 10+col*12,-(m_imgHeight+22)+row*18,charact );
}   }   }
