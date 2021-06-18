/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "ili9341.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"


Component* Ili9341::construct( QObject* parent, QString type, QString id )
{
    return new Ili9341( parent, type, id );
}

LibraryItem* Ili9341::libraryItem()
{
    return new LibraryItem(
        tr( "Ili9341" ),
        tr( "Displays" ),
        "ili9341.png",
        "Ili9341",
        Ili9341::construct );
}

Ili9341::Ili9341( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eClockedDevice( id )
       , m_pinCS (  270, QPoint(-56, 184), id+"-PinCS"  , 0, this, input )
       , m_pinRst(  270, QPoint(-48, 184), id+"-PinRst" , 0, this, input )
       , m_pinDC (  270, QPoint(-40, 184), id+"-PinDC"  , 0, this, input )
       , m_pinMosi( 270, QPoint(-32, 184), id+"-PinMosi" , 0, this, input )
       , m_pinSck(  270, QPoint(-24, 184), id+"-PinSck" , 0, this, input )
       //, m_pinMiso( 270, QPoint(-16, 184), id+"-PinMiso" , 0, this )
{
    m_graphical = true;
    
    m_area = QRectF( -126, -168, 252, 344 );

    m_pinCS.setLabelText(   " CS" );
    m_pinCS.setInputHighV( 2.31 );
    m_pinCS.setInputLowV( 0.99 );
    m_pinRst.setLabelText(  " Rst" );
    m_pinRst.setInputHighV( 2.31 );
    m_pinRst.setInputLowV( 0.99 );
    m_pinDC.setLabelText(   " D/C" );
    m_pinDC.setInputHighV( 2.31 );
    m_pinDC.setInputLowV( 0.99 );
    m_pinMosi.setLabelText( " Mosi" );
    m_pinMosi.setInputHighV( 2.31 );
    m_pinMosi.setInputLowV( 0.99 );
    m_pinSck.setLabelText(  " SCK" );
    m_pinSck.setInputHighV( 2.31 );
    m_pinSck.setInputLowV( 0.99 );
    //m_pinMiso.setLabelText( " Miso" );

    m_pin.resize( 5 );
    m_pin[0] = &m_pinCS;
    m_pin[1] = &m_pinRst;
    m_pin[2] = &m_pinDC;
    m_pin[3] = &m_pinMosi;
    m_pin[4] = &m_pinSck;
    //m_pin[5] = &m_pinMiso;

    m_clkPin = &m_pinSck;

    m_pdisplayImg = new QImage( 240, 320, QImage::Format_RGB888 );

    Simulator::self()->addToUpdateList( this );
    
    setLabelPos(-32,-180, 0);
    setShowId( true );
    
    initialize();
}

Ili9341::~Ili9341(){}

void Ili9341::stamp()
{
    m_pinSck.changeCallBack( this ); // Register for Scl changes callback
    m_pinCS.changeCallBack( this ); // Register for CS changes callback
    m_pinRst.changeCallBack( this ); // Register for Rst changes callback
}

void Ili9341::initialize()
{
    clearDDRAM();
    reset() ;
    updateStep();
}

void Ili9341::voltChanged()                 // Called when En Pin changes
{
    bool ret = false;
    if( !m_pinRst.getInpState() )            // Reset Pin is Low
    {
        reset();
        ret = true;
    }
    if( m_pinCS.getInpState() )            // Cs Pin High: Lcd not selected
    {
        m_rxReg = 0;                       // Initialize serial buffer
        m_inBit  = 0;
        ret = true;
    }
    updateClock();
    if( m_clkState != Clock_Rising ) ret = true;
    if( ret ) return;

    m_rxReg &= ~1; //Clear bit 0
    if( m_pinMosi.getInpState() ) m_rxReg |= 1;

    if( m_inBit >= 7 )
    {
        if( m_pinDC.getInpState() )       // Write Data
        {
            if( m_readBytes == 0 )        // Write DDRAM
            {
                if( m_inByte == 0 ) m_data = m_rxReg; // Lower byte
                else  m_data = (m_data<<8)+m_rxReg;   // High byte

                m_inByte++;
                if( m_inByte >= m_dataBytes )       // 16/18 bits ready
                {
                    m_inByte = 0;
                    //qDebug() << "Ili9341::setVChanged"<< m_addrX<<m_addrY<< m_rxReg;

                    uint blue=0, green=0, red=0;
                    if( m_dataBytes == 2 ) // 16 bits format: RRRRRGGGGGGBBBBB
                    {
                        blue  = (m_data & 0b0000000000011111)<<3;
                        green = (m_data & 0b0000011111100000)<<5;
                        red   = (m_data & 0b1111100000000000)<<8;
                    }
                    else // 18 bits format: RRRRRR00GGGGGG00BBBBBB00
                    {
                        blue  = (m_data & 0b000000000000000011111100);
                        green = (m_data & 0b000000001111110000000000);
                        red   = (m_data & 0b111111000000000000000000);
                    }

                    m_aDispRam[m_addrX][m_addrY] = red+green+blue;
                    incrementPointer();
                }
            }
            else proccessCommand();       // Write Command Parameter
        }
        else proccessCommand();           // Write Command
        m_inBit = 0;
    }
    else
    {
        m_rxReg <<= 1;
        m_inBit++;
    }
}

void Ili9341::proccessCommand()
{
    if( m_readBytes > 0 )
    {
        switch( m_lastCommand )
        {
            case 0x2A:
            {
                if( m_readBytes == 4 ) m_startX  = m_rxReg<<8;
                if( m_readBytes == 3 )
                {
                    m_startX += m_rxReg;
                    m_addrX = m_startX;
                }
                if( m_readBytes == 2 ) m_endX  = m_rxReg<<8;
                else                   m_endX += m_rxReg;

                if     ( m_startX < 0 )  m_startX = 0;
                else if( m_startX >239 ) m_startX = 239;
                if     ( m_endX < 0 )    m_endX = 0;
                else if( m_endX >239 )   m_endX = 239;
            }break;
            case 0x2B:
            {
                if( m_readBytes == 4 ) m_startY  = m_rxReg<<8;
                if( m_readBytes == 3 )
                {
                    m_startY += m_rxReg;
                    m_addrY = m_startY;
                }
                if( m_readBytes == 2 ) m_endY  = m_rxReg<<8;
                else                   m_endY += m_rxReg;

                if     ( m_startY < 0 )  m_startY = 0;
                else if( m_startY >319 ) m_startY = 319;
                if     ( m_endY < 0 )  m_endY = 0;
                else if( m_endY >319 ) m_endY = 319;
            }break;
            case 0x3A:
            {
                int mode = (m_rxReg>>4) & 1;
                m_dataBytes = mode ? 2 : 3;
            }
        }

        m_readBytes--;
        return;
    }
    m_lastCommand = m_rxReg;

    switch( m_rxReg )
    {
        case 0x01: //   Software Reset
        {
            clearLcd();
            //Clear variables
        }break;
        case 0x04: m_readBytes = 4; break; // Read Display identification information
        case 0x09: m_readBytes = 5; break; // Read Display Status
        case 0x0A: m_readBytes = 2; break; // Read Display Power Mode
        case 0x0B: m_readBytes = 2; break; // Read Display MADCTL
        case 0x0C: m_readBytes = 2; break; // Read Display Pixel Format
        case 0x0D: m_readBytes = 2; break; // Read Display Image Format
        case 0x0E: m_readBytes = 2; break; // Read Display Signal Mode
        case 0x0F: m_readBytes = 2; break; // Read Display Sek-Diagnostic Result
        //case 0x10: // Enter Sleep Mode
        //case 0x11: // Sleep Out
        //case 0x12: // Partial Mode On
        //case 0x13: // Normal Mode On
        case 0x20: m_dispInv = false; break; // Display Inversion Off
        case 0x21: m_dispInv = true; break;  // Display Inversion On
        case 0x26: m_readBytes = 1; break;   // Gamma Set
        case 0x28: m_dispOn = false; break;  // Display Off
        case 0x29: m_dispOn = true; break;   // Display On
        case 0x2A: m_readBytes = 4; break;   // Column Address Set
        case 0x2B: m_readBytes = 4; break;   // Page Address Set
        //case 0x2C: // Memory Write
        case 0x2D: m_readBytes = 128; break; // Color Set
        //case 0x2E: // Memory Read
        case 0x30: m_readBytes = 4; break;   // Partial Area
        case 0x33: m_readBytes = 6; break;   // Vertical Scrolling Definition
        //case 0x34: // Tearing Effect Line Off
        //case 0x35: // Tearing Effect Line On
        case 0x36: m_readBytes = 1; break;   // Memory Access Control
        case 0x37: m_readBytes = 1; break;   // Vertical Scrolling Start Address (2 params in datasheet??:
        //case 0x38: // Idle Mode Off
        //case 0x39: // Idle Mode On
        case 0x3A: m_readBytes = 1; break;   // COLMOD: Pixel Formay Set
        //case 0x3C: // Write Memory Continue
        //case 0x3E: // Read Memory Continue
        case 0x44: m_readBytes = 2; break;   // Set Tear Scanline
        case 0x45: m_readBytes = 2; break;   // Get Scanline
        case 0x51: m_readBytes = 2; break;   // Write Display Brightness
        case 0x52: m_readBytes = 2; break;   // Read Display Brightness
        case 0x53: m_readBytes = 1; break;   // Write CTRL Display
        case 0x54: m_readBytes = 2; break;   // Write CTRL Display
        case 0x55: m_readBytes = 1; break;   // Write Content Adaptive Brightness Control
        case 0x56: m_readBytes = 2; break;   // Read Content Adaptive Brightness Control
        case 0x5E: m_readBytes = 1; break;   // Write CABC Minimum Brightness
        case 0x5F: m_readBytes = 2; break;   // Read CABC Minimum Brightness

        case 0xB0: m_readBytes = 1; break;   // RGB Interface Signal Control
        case 0xB1: m_readBytes = 2; break;   // Frame Rate Control (In Normal Mode/Full Colors:
        case 0xB2: m_readBytes = 2; break;   // Frame Rate Control (In Idle Mode/8 colors:
        case 0xB3: m_readBytes = 2; break;   // Frame Rate control (In Partial Mode/Full Colors:
        case 0xB4: m_readBytes = 1; break;   // Display Inversion Control
        case 0xB5: m_readBytes = 4; break;   // Blanking Porch Control
        case 0xB6: m_readBytes = 3; break;   // Display Function Control
        case 0xB7: m_readBytes = 1; break;   // Entry Mode Set
        case 0xB8: m_readBytes = 1; break;   // Backlight Control 1
        case 0xB9: m_readBytes = 1; break;   // Backlight Control 2
        case 0xBA: m_readBytes = 1; break;   // Backlight Control 3
        case 0xBB: m_readBytes = 1; break;   // Backlight Control 4
        case 0xBC: m_readBytes = 1; break;   // Backlight Control 5
        case 0xBE: m_readBytes = 1; break;   // Backlight Control 7
        case 0xBF: m_readBytes = 1; break;   // Backlight Control 8
        case 0xC0: m_readBytes = 1; break;   // Power Control 1
        case 0xC1: m_readBytes = 1; break;   // Power Control 2
        case 0xC5: m_readBytes = 2; break;   // VCOM Control 1
        case 0xC7: m_readBytes = 1; break;   // VCOM Control 2
        case 0xD0: m_readBytes = 2; break;   // NV Memory Write
        case 0xD1: m_readBytes = 3; break;   // NV Memory Protection Key
        case 0xD2: m_readBytes = 3; break;   // NV Memory Status Read
        case 0xD3: m_readBytes = 4; break;   // Read ID4
        case 0xDA: m_readBytes = 2; break;   // Read ID1
        case 0xDB: m_readBytes = 2; break;   // Read ID2
        case 0xDC: m_readBytes = 2; break;   // Read ID3
        case 0xE0: m_readBytes = 15; break;  // Positive Gamma Correction
        case 0xE1: m_readBytes = 15; break;  // Negative Gamma Correction
        case 0xE2: m_readBytes = 16; break;  // Digital Gamma Control 1
        case 0xE3: m_readBytes = 64; break;  // Digital Gamma Control 2
        case 0xF6: m_readBytes = 3; break;   // Interface Control

        // Misterious Commands in Adafruit_ILI9341.cpp initcmd[]
        case 0xEF: m_readBytes = 3; break; //, 0x03, 0x80, 0x02,
        case 0xCF: m_readBytes = 3; break; //, 0x00, 0xC1, 0x30,
        case 0xED: m_readBytes = 4; break; //, 0x64, 0x03, 0x12, 0x81,
        case 0xE8: m_readBytes = 3; break; //, 0x85, 0x00, 0x78,
        case 0xCB: m_readBytes = 5; break; //, 0x39, 0x2C, 0x00, 0x34, 0x02,
        case 0xF7: m_readBytes = 1; break; //, 0x20,
        case 0xEA: m_readBytes = 2; break; //, 0x00, 0x00,
        case 0xF2: m_readBytes = 1; break; //, 0x00, 0x00,
    }
    //qDebug() << "Ili9341::proccessCommand: " << command;
}

void Ili9341::clearLcd() 
{
    m_pdisplayImg->fill(0);
}

void Ili9341::clearDDRAM() 
{
    for( int row=0; row<320; row++ )
        for( int col=0; col<240; col++ )
            m_aDispRam[col][row] = 0;
}

void Ili9341::incrementPointer() 
{
    /*if( m_addrMode == VERT_ADDR_MODE )
    {
        m_addrY++;
        if( m_addrY > m_endY )
        {
            m_addrY = m_startY;
            m_addrX++;
        }
        if( m_addrX > m_endX )
        {
            m_addrX = m_startX;
        }
    }
    else*/
    {
        m_addrX++;
        if( m_addrX > m_endX )
        {
            m_addrX = m_startX;
            //if( m_addrMode == HORI_ADDR_MODE )
                m_addrY++;
        }
        //if( m_addrMode == HORI_ADDR_MODE )
        {
            if( m_addrY > m_endY )
            {
                m_addrY = m_startY;
            }
        }
    }
}

void Ili9341::reset() 
{
    m_addrX  = 0;
    m_addrY  = 0;
    m_startX = 0;
    m_endX   = 239;
    m_startY = 0;
    m_endY   = 319;
    m_inBit  = 0;
    m_inByte = 0;

    m_scrollStartPage  = 0;
    m_scrollEndPage    = 7;
    m_scrollInterval   = 5;
    m_scrollVertOffset = 0;

    m_startLin = 0;
    m_readBytes = 0;
    m_dataBytes = 2; //16bit mode

    m_dispOn   = false;
    //m_dispFull = false;
    m_dispInv  = false;

    m_scroll  = false;
    m_scrollR = false;
    m_scrollV = false;

    //m_reset = true;

    clearLcd();
}

void Ili9341::remove()
{
    delete m_pdisplayImg;
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

void Ili9341::updateStep()
{
    //if( !m_dispOn ) m_pdisplayImg->fill(0);               // Display Off
    //else
    //{
        /*if( m_scroll )
        {
            m_scrollCount--;
            if( m_scrollCount <= 0 )
            {
                m_scrollCount = m_scrollInterval;

                for( int row=m_scrollStartPage; row<=m_scrollEndPage; row++ )
                {
                    unsigned char start = m_aDispRam[0][row];
                    unsigned char end   = m_aDispRam[m_endX][row];

                    for( int col=0; col<=m_endX; col++ )
                    {

                        if( m_scrollR )
                        {
                            int c = m_endX-col;

                            if( c < m_endX ) m_aDispRam[c][row] = m_aDispRam[c-1][row];
                            if( col == 0 )  m_aDispRam[0][row]   = end;
                        }
                        else
                        {
                            if( col < m_endX )  m_aDispRam[col][row] = m_aDispRam[col+1][row];
                            if( col == m_endX ) m_aDispRam[col][row] = start;
                        }
                    }

                }
            }

        }*/
        /*for( int row=0; row<=319; row++ )
        {
            for( int col=0; col<=239; col++ )
            {
                unsigned int pixel;
                //if( m_dispFull ) pixel = 0xFFFF;        // Display fully On
                //else
                    pixel = m_aDispRam[col][row];

                //if( m_dispInv ) abyte = ~abyte;         // Display Inverted

                m_pdisplayImg->setPixel(col,row, QColor(pixel).rgb() );
            }
        }
    }*/
    update();
}

void Ili9341::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( !m_dispOn ) m_pdisplayImg->fill(0);               // Display Off
    else
    {
        for( int row=0; row<=319; row++ )
        {
            for( int col=0; col<=239; col++ )
            {
                unsigned int pixel;
                //if( m_dispFull ) pixel = 0xFFFF;        // Display fully On
                //else
                    pixel = m_aDispRam[col][row];

                //if( m_dispInv ) abyte = ~abyte;         // Display Inverted

                m_pdisplayImg->setPixel(col,row, QColor(pixel).rgb() );
            }
        }
    }

    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );
    p->drawImage(-120,-162,*m_pdisplayImg );
}

#include "moc_ili9341.cpp"
