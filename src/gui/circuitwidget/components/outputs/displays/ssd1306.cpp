/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "ssd1306.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

#include "doubleprop.h"
#include "stringprop.h"
#include "intprop.h"

Component* Ssd1306::construct( QObject* parent, QString type, QString id )
{ return new Ssd1306( parent, type, id ); }

LibraryItem* Ssd1306::libraryItem()
{
    return new LibraryItem(
        "SSD1306",
        "Displays",
        "ssd1306.png",
        "Ssd1306",
        Ssd1306::construct );
}

Ssd1306::Ssd1306( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , TwiModule( id )
       //, m_pinRst( 270, QPoint(-32, 48), id+"-PinRst" , 0, this )
       //, m_pinDC ( 270, QPoint(-24, 48), id+"-PinDC"  , 0, this )
       //, m_pinCS ( 270, QPoint(-16, 48), id+"-PinCS"  , 0, this )
{
    m_graphical = true;
    m_width = 128;
    m_height = 64;
    m_rows   = 8;
    m_area = QRectF( -70, -m_height/2-16, m_width+12, m_height+24 );
    m_address = m_cCode = 0b00111100; // 0x3A - 60

    m_enumUids = QStringList()
        << "White"
        << "Blue"
        << "Yellow";

    m_enumNames = QStringList()
        << tr("White")
        << tr("Blue")
        << tr("Yellow");

    m_pin.resize( 2 );
    m_clkPin = new IoPin( 270, QPoint(-48, 48), id+"-PinSck" , 0, this, openCo );
    m_clkPin->setLabelText( "SCL" );
    m_pin[0] = m_clkPin;
    TwiModule::setSclPin( m_clkPin );

    m_pinSda = new IoPin( 270, QPoint(-40, 48), id+"-PinSda" , 0, this, openCo );
    m_pin[1] = m_pinSda;
    m_pinSda->setLabelText( "SDA" );
    TwiModule::setSdaPin( m_pinSda );

    //m_pinRst.setLabelText( "Res" );
    //m_pinDC.setLabelText(  "DC" );
    //m_pinCS.setLabelText(  "CS" );

    m_pdisplayImg = new QImage( 128, 64, QImage::Format_MonoLSB );
    m_pdisplayImg->setColor( 0, qRgb(0, 0, 0));
    m_pdisplayImg->setColor( 1, qRgb(255, 255, 255) );
    m_dColor = White;
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos(-32,-60, 0);
    setShowId( true );
    
    Ssd1306::initialize();

    addPropGroup( { tr("Main"), {
new StrProp <Ssd1306>("Color"       ,tr("Color")        ,""           ,this,&Ssd1306::colorStr,&Ssd1306::setColorStr,0,"enum" ),
new IntProp <Ssd1306>("Width"       ,tr("Width")        ,tr("_Pixels"),this,&Ssd1306::width,   &Ssd1306::setWidth,0,"uint" ),
new IntProp <Ssd1306>("Height"      ,tr("Height")       ,tr("_Pixels"),this,&Ssd1306::height,  &Ssd1306::setHeight,0,"uint" ),
new IntProp <Ssd1306>("Control_Code",tr("I2C Address")  ,""           ,this,&Ssd1306::cCode,   &Ssd1306::setCcode,0,"uint" ),
new DoubProp<Ssd1306>("Frequency"   ,tr("I2C Frequency"),"_KHz"       ,this,&Ssd1306::freqKHz, &Ssd1306::setFreqKHz ),
    }, groupNoCopy} );
}
Ssd1306::~Ssd1306(){}

void Ssd1306::initialize()
{
    TwiModule::initialize();

    m_continue = false;
    m_command = false;
    m_data = false;
    m_addrMode = HORI_ADDR_MODE;

    clearDDRAM();
    reset() ;
    Ssd1306::updateStep();
}

void Ssd1306::stamp()
{
    setMode( TWI_SLAVE );
}

void Ssd1306::updateStep()
{
    if     ( !m_dispOn )  m_pdisplayImg->fill(0);  // Display Off
    else if( m_dispFull ) m_pdisplayImg->fill(255); // Display fully On
    else{
        if( m_scroll )
        {
            m_scrollCount--;
            if( m_scrollCount <= 0 )
            {
                m_scrollCount = m_scrollInterval;

                for( int row=m_scrollStartPage; row<=m_scrollEndPage; row++ )
                {
                    unsigned char start = m_aDispRam[0][row];
                    unsigned char end   = m_aDispRam[127][row];

                    for( int col=0; col<128; col++ )
                    {
                        if( m_scrollR )
                        {
                            int c = 127-col;
                            if( c < 127 ) m_aDispRam[c][row] = m_aDispRam[c-1][row];
                            if( col == 0 )  m_aDispRam[0][row]   = end;
                        }else{
                            if( col < 127 )  m_aDispRam[col][row] = m_aDispRam[col+1][row];
                            if( col == 127 ) m_aDispRam[col][row] = start;
        }   }   }   }   }
        for( int row=0; row<8; row++ )
        {
            for( int col=0; col<128; col++ )
            {
                unsigned char abyte = m_aDispRam[col][row];

                if( m_dispInv ) abyte = ~abyte;         // Display Inverted

                for( int bit=0; bit<8; bit++ )
                {
                    m_pdisplayImg->setPixel(col,row*8+bit,(abyte & 1) );
                    abyte >>= 1;
    }   }   }   }
    update();
}

void Ssd1306::reset()
{
    m_cdr = 1;
    m_mr  = 63;
    m_fosc = 370000;
    m_frm = m_fosc/(m_cdr*54*m_mr);

    m_addrX  = 0;
    m_addrY  = 0;
    m_startX = 0;
    m_endX   = 127;
    m_startY = 0;
    m_endY   = m_rows-1;

    m_scrollStartPage  = 0;
    m_scrollEndPage    = 7;
    m_scrollInterval   = 5;
    m_scrollVertOffset = 0;

    m_startLin = 0;
    m_readBytes = 0;

    m_dispOn   = false;
    m_dispFull = false;
    m_dispInv  = false;

    m_scroll  = false;
    m_scrollR = false;
    m_scrollV = false;

    clearLcd();
}

void Ssd1306::I2Cstop()
{
    TwiModule::I2Cstop();

    m_command = false;
    m_data    = false;
    m_continue = false;
}

void Ssd1306::readByte()
{
    TwiModule::readByte();

    if( !m_command && !m_data )
    {
        if( (m_rxReg & 0b00111111) == 0 ) // Control Byte
        {
            int co = m_rxReg & 0b10000000;
            if( co == 0 ) m_continue = true;
            else          m_continue = false;

            int cd = m_rxReg & 0b01111111;
            if( cd == 0 ) m_command = true;// 0 Command Byte
            else          m_data    = true;// 64 Data Byte
    }   }
    else{                               // Data Byte
        if     ( m_command ) proccessCommand();
        else if( m_data )    writeData();

        if( !m_continue )
        {
            m_command = false;
            m_data    = false;
}   }   }

void Ssd1306::writeData()
{
    m_aDispRam[m_addrX][m_addrY] = m_rxReg;
    incrementPointer();
}

void Ssd1306::proccessCommand()
{
    if( m_readBytes > 0 )
    {
        if( m_lastCommand == 0x20 ) m_addrMode = m_rxReg;
        else if( m_lastCommand == 0x21 ) // 21 33 Set Column Address (Start-End)
        {
            if( m_readBytes == 2 ) m_startX = m_rxReg & 0x7F; // 0b01111111
            else                   m_endX   = m_rxReg & 0x7F; // 0b01111111
        }
        else if( m_lastCommand == 0x22 ) // 22 34 Set Page Address (Start-End)
        {
            if( m_readBytes == 2 ) m_startY = m_rxReg & 0x07; // 0b00000111
            else{                                             // 0b00000111
                m_endY = m_rxReg & 0x07;
                if( m_endY > m_rows-1 ) m_endY = m_rows-1;
            }
        }
        else if( m_lastCommand == 0x26   // 26 36 Continuous Horizontal Scroll Setup
              || m_lastCommand == 0x27
              || m_lastCommand == 0x29   // 29-2A 38 39 Continuous Vertical and Horizontal Scroll Setup
              || m_lastCommand == 0x2A )
        {
            int byte = 6-m_readBytes;
            int value = m_rxReg & 0x07; // 0b00000111

            m_scrollV = false;

            if     ( m_lastCommand == 0x26 ) m_scrollR = true;
            else if( m_lastCommand == 0x27 ) m_scrollR = false;
            else if( m_lastCommand == 0x29 )
            {
                byte = 5-m_readBytes;
                m_scrollV = true;
                m_scrollR = true;
            }
            else if( m_lastCommand == 0x2A )
            {
                byte = 5-m_readBytes;
                m_scrollV = true;
                m_scrollR = false;
            }
            if     ( byte == 1 ) m_scrollStartPage =  value;
            else if( byte == 2 )
            {
                if     ( value == 0 ) m_scrollInterval = 5;
                else if( value == 1 ) m_scrollInterval = 64;
                else if( value == 2 ) m_scrollInterval = 128;
                else if( value == 3 ) m_scrollInterval = 256;
                else if( value == 4 ) m_scrollInterval = 3;
                else if( value == 5 ) m_scrollInterval = 4;
                else if( value == 6 ) m_scrollInterval = 25;
                else if( value == 7 ) m_scrollInterval = 2;
            }
            else if( byte == 3 ) m_scrollEndPage =  value;
            else if( byte == 4 )
            {
                m_scrollVertOffset = m_rxReg & 0x3F; // 0b00111111
            }
            //qDebug() << m_lastCommand << byte << m_scrollR << m_scrollV << m_scrollInterval << m_scrollStartPage << m_scrollEndPage<<m_scrollVertOffset;
        }
        else if( m_lastCommand == 0xA3 ) // A3 163 Set Vertical Scroll Area
        {
            ; /// TODO
        }
        m_readBytes--;
        return;
    }
    m_lastCommand = m_rxReg;

    if( m_rxReg < 0x10 ) // 00-0F 0-15 Set Lower Colum Start Address for Page Addresing mode
    {
        m_addrX = (m_addrX & ~0xF) | (m_rxReg & 0xF);
    }
    else if( m_rxReg < 0x20 ) // 10-1F 16-31 Set Higher Colum Start Address for Page Addresing mode
    {
        m_addrX = (m_addrX & 0xF) | ((m_rxReg & 0xF) << 4);
    }
    else if( m_rxReg == 0x20 ) m_readBytes = 1; // 20 32 Set Memory Addressing Mode
    else if( m_rxReg == 0x21 ) m_readBytes = 2; // 21 33 Set Column Address (Start-End)
    else if( m_rxReg == 0x22 ) m_readBytes = 2; // 22 34 Set Page Address (Start-End)

    else if( m_rxReg == 0x26 ) m_readBytes = 6; // 26 36 Continuous Horizontal Right Scroll Setup
    else if( m_rxReg == 0x27 ) m_readBytes = 6; // 27 37 Continuous Horizontal Left Scroll Setup
    else if( m_rxReg == 0x29 ) m_readBytes = 5; // 29-2A 38 39 Continuous Vertical and Horizontal Scroll Setup
    else if( m_rxReg == 0x2A ) m_readBytes = 5;

    else if( m_rxReg == 0x2E ) m_scroll = false; // 0b00101110 // 0x2E 46    Deactivate scroll
    else if( m_rxReg == 0x2F )
    {
        m_scroll = true;  // 0b00101111 // 0x2F 47    Activate scroll
        m_scrollCount = m_scrollInterval/5;
        //qDebug() << "Activate Scroll" << m_scrollCount<<"\n";
    }

    else if( (m_rxReg>=0x40) && (m_rxReg<=0x7F) ) // 0b01xxxxxx 40-7F 64-127 Set Display Start Line
    {
        m_startLin = m_rxReg & 0x3F; // 0b00111111
    }

    else if( m_rxReg == 0x81 ) m_readBytes = 1; // 81 129 Set Contrast Control

    else if( m_rxReg == 0x8D ) m_readBytes = 1; // 8D 141 Charge Pump

    // A0-A1 160-161 Set Segment Re-map

    else if( m_rxReg == 0xA3 ) m_readBytes = 2;     // A3 163 Set Vertical Scroll Area
    else if( m_rxReg == 0xA4 ) m_dispFull = false;  // A4-A5 164-165 Entire Display ON
    else if( m_rxReg == 0xA5 ) m_dispFull = true;
    else if( m_rxReg == 0xA6 ) m_dispInv = false;   // A6-A7 166-167 Set Normal/inverse Display
    else if( m_rxReg == 0xA7 ) m_dispInv = true;
    else if( m_rxReg == 0xA8 ) m_readBytes = 1;     // A8 168 Set Multiplex Ratio

    else if( m_rxReg == 0xAE ) reset();             // 174 // AE-AF Set Display ON/OFF
    else if( m_rxReg == 0xAF ) m_dispOn = true;     // 175

    else if( (m_rxReg>=0xB0) && (m_rxReg<=0xB7) )   // B0-B7 176-183 Set Page Start Address for Page Addresing mode
    {
        m_addrY = m_rxReg & 0x07; // 0b00000111
    }
    // C0-C8 192-200 Set COM Output Scan Direction

    else if( m_rxReg == 0xD3 ) m_readBytes = 1; // D3 211 Set Display Offset

    else if( m_rxReg == 0xD5 ) m_readBytes = 1; // D5 213 Set Display Clock Divide Ratio/Oscillator Frequency

    else if( m_rxReg == 0xD9 ) m_readBytes = 1; // D9 217 Set Precharge
    else if( m_rxReg == 0xDA ) m_readBytes = 1; // DA 218 Set COM Pins Hardware Configuration
    else if( m_rxReg == 0xDB ) m_readBytes = 1; // DB 219 SET VCOM DETECT
}

void Ssd1306::clearLcd() 
{
    m_pdisplayImg->fill(0);
}

void Ssd1306::clearDDRAM() 
{
    for( int row=0; row<8; row++ )
        for( int col=0; col<128; col++ )
            m_aDispRam[col][row] = 0;
}

void Ssd1306::incrementPointer() 
{
    if( m_addrMode == VERT_ADDR_MODE )
    {
        m_addrY++;
        if( m_addrY > m_endY )
        {
            m_addrY = m_startY;
            m_addrX++;
        }
        if( m_addrX > m_endX ) m_addrX = m_startX;
    }else{
        m_addrX++;
        if( m_addrX > m_endX )
        {
            m_addrX = m_startX;
            if( m_addrMode == HORI_ADDR_MODE ) m_addrY++;
        }
        if( m_addrMode == HORI_ADDR_MODE )
        {
            if( m_addrY > m_endY ) m_addrY = m_startY;
}   }   }

void Ssd1306::remove()
{
    delete m_pdisplayImg;
    Component::remove();
}

void Ssd1306::setColorStr( QString color )
{
    int c = getEnumIndex(  color );
    m_dColor = (dispColor)c;

    if( c == White )  m_pdisplayImg->setColor( 1, qRgb(245, 245, 245) );
    if( c == Blue  )  m_pdisplayImg->setColor( 1, qRgb(200, 200, 255) );
    if( c == Yellow ) m_pdisplayImg->setColor( 1, qRgb(245, 245, 100) );

    if( m_showVal && (m_showProperty == "Color") )
        setValLabelText( m_enumNames.at( c ) );
}

void Ssd1306::setWidth( int w )
{
    if     ( w > 128 ) w = 128;
    else if( w < 32  ) w = 32;
    if( m_width == w ) return;
    m_width = w;
    updateSize();
}

void Ssd1306::setHeight( int h )
{
    if     ( h > 64 ) h = 64;
    else if( h < 16 ) h = 16;
    m_rows = h/8;
    m_height = m_rows*8;
    updateSize();
}

void Ssd1306::updateSize()
{
    m_area = QRectF( -70, -m_height/2-16, m_width+12, m_height+24 );
    m_clkPin->setPos( QPoint(-48, m_height/2+16) );
    m_clkPin->isMoved();
    m_pinSda->setPos( QPoint(-40, m_height/2+16) );
    m_pinSda->isMoved();
    Circuit::self()->update();
}

void Ssd1306::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    
    p->setBrush( QColor( 50, 70, 100 ) );
    p->drawRoundedRect( m_area, 2, 2 );
    p->drawImage(-64,-m_height/2-10, *m_pdisplayImg, 0, 0, m_width, m_height );
}
