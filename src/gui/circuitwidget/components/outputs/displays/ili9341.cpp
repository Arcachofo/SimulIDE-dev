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
       , eLogicDevice( id )
       , m_pinCS (  270, QPoint(-56, 184), id+"-PinCS"  , 0, this )
       , m_pinRst(  270, QPoint(-48, 184), id+"-PinRst" , 0, this )
       , m_pinDC (  270, QPoint(-40, 184), id+"-PinDC"  , 0, this )
       , m_pinMosi( 270, QPoint(-32, 184), id+"-PinMosi" , 0, this )
       , m_pinSck(  270, QPoint(-24, 184), id+"-PinSck" , 0, this )
       //, m_pinMiso( 270, QPoint(-16, 184), id+"-PinMiso" , 0, this )
{
    m_graphical = true;
    
    m_area = QRectF( -126, -168, 252, 344 );

    m_pinCS.setLabelText(   " CS" );
    m_pinRst.setLabelText(  " Rst" );
    m_pinDC.setLabelText(   " D/C" );
    m_pinMosi.setLabelText( " Mosi" );
    m_pinSck.setLabelText(  " SCK" );
    //m_pinMiso.setLabelText( " Miso" );

    m_pin.resize( 5 );
    m_pin[0] = &m_pinCS;
    m_pin[1] = &m_pinRst;
    m_pin[2] = &m_pinDC;
    m_pin[3] = &m_pinMosi;
    m_pin[4] = &m_pinSck;
    //m_pin[5] = &m_pinMiso;

    eLogicDevice::createInput( &m_pinCS );
    eLogicDevice::createInput( &m_pinRst );
    eLogicDevice::createInput( &m_pinDC );
    eLogicDevice::createInput( &m_pinMosi );
    eLogicDevice::createClockPin( &m_pinSck );

    m_pdisplayImg = new QImage( 240, 320, QImage::Format_RGB888 );

    Simulator::self()->addToUpdateList( this );
    
    setLabelPos(-32,-180, 0);
    setShowId( true );
    
    initialize();
}

Ili9341::~Ili9341(){}

void Ili9341::stamp()
{
    eNode* enode = m_pinSck.getEnode();// Register for Scl changes callback
    if( enode ) enode->voltChangedCallback( this );

    enode = m_pinCS.getEnode();// Register for CS changes callback
    if( enode ) enode->voltChangedCallback( this ); 
    
    enode = m_pinRst.getEnode();       // Register for Rst changes callback
    if( enode ) enode->voltChangedCallback( this ); 
}

void Ili9341::initialize()
{
    //m_addrMode = 0;

    clearDDRAM();
    //clearLcd();
    reset() ;
    updateStep();
}

void Ili9341::voltChanged()                 // Called when En Pin changes
{
    if( !eLogicDevice::getInputState(1) )  // Reset Pin is Low
    {
        reset();
        return;
    }
    if( eLogicDevice::getInputState(0) )   // Cs Pin High: Lcd not selected
    {
        m_rxReg = 0;                       // Initialize serial buffer
        m_inBit = 0;
        return;
    }
    if( eLogicDevice::getClockState() != Clock_Rising ) return;

    m_rxReg &= ~1; //Clear bit 0

    if( eLogicDevice::getInputState(3) ) m_rxReg |= 1; // Pin Mosi

    if( m_inBit == 7 )
    {
        if( eLogicDevice::getInputState(2) )       // Pin DC Write Data
        {
            if( m_readBytes == 0 )        // Write DDRAM
            {
                if( m_inByte == 0 ) m_data = m_rxReg; // Lower byte
                else  m_data = (m_data<<8)+m_rxReg;   // High byte

                m_inByte++;
                if( m_inByte > 1 )       // 16 bits ready
                {
                    m_inByte = 0;
                    //qDebug() << "Ili9341::setVChanged"<< m_addrX<<m_addrY<< m_rxReg;

                    // 16 bits format: RRRRRGGGGGGBBBBB
                    int blue  = (m_data & 0b00011111)<<3;
                    int green = (m_data & 0b11111100000)<<5;
                    int red   = (m_data & 0b1111100000000000)<<8;

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
        if( m_lastCommand == 0x2A )
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
            if     ( m_endX < 0 )  m_endX = 0;
            else if( m_endX >239 ) m_endX = 239;
        }
        else if( m_lastCommand == 0x2B )
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
        }

        m_readBytes--;
        return;
    }
    m_lastCommand = m_rxReg;

    if( m_rxReg == 0x01 ) //   Software Reset
    {
        clearLcd();
        //Clear variables
    }
    else if( m_rxReg == 0x04 ) m_readBytes = 4; // Read Display identification information
    else if( m_rxReg == 0x09 ) m_readBytes = 5; // Read Display Status
    else if( m_rxReg == 0x0A ) m_readBytes = 2; // Read Display Power Mode
    else if( m_rxReg == 0x0B ) m_readBytes = 2; // Read Display MADCTL
    else if( m_rxReg == 0x0C ) m_readBytes = 2; // Read Display Pixel Format
    else if( m_rxReg == 0x0D ) m_readBytes = 2; // Read Display Image Format
    else if( m_rxReg == 0x0E ) m_readBytes = 2; // Read Display Signal Mode
    else if( m_rxReg == 0x0F ) m_readBytes = 2; // Read Display Sek-Diagnostic Result
    //else if( m_rxReg == 0x10 ) // Enter Sleep Mode
    //else if( m_rxReg == 0x11 ) // Sleep Out
    //else if( m_rxReg == 0x12 ) // Partial Mode On
    //else if( m_rxReg == 0x13 ) // Normal Mode On
    else if( m_rxReg == 0x20 ) m_dispInv = false; // Display Inversion Off
    else if( m_rxReg == 0x21 ) m_dispInv = true;  // Display Inversion On
    else if( m_rxReg == 0x26 ) m_readBytes = 1;   // Gamma Set
    else if( m_rxReg == 0x28 ) m_dispOn = false;  // Display Off
    else if( m_rxReg == 0x29 ) m_dispOn = true;   // Display On
    else if( m_rxReg == 0x2A ) m_readBytes = 4;   // Column Address Set
    else if( m_rxReg == 0x2B ) m_readBytes = 4;   // Page Address Set
    //else if( m_rxReg == 0x2C ) // Memory Write
    else if( m_rxReg == 0x2D ) m_readBytes = 128; // Color Set
    //else if( m_rxReg == 0x2E ) // Memory Read
    else if( m_rxReg == 0x30 ) m_readBytes = 4;   // Partial Area
    else if( m_rxReg == 0x33 ) m_readBytes = 6;   // Vertical Scrolling Definition
    //else if( m_rxReg == 0x34 ) // Tearing Effect Line Off
    //else if( m_rxReg == 0x35 ) // Tearing Effect Line On
    else if( m_rxReg == 0x36 ) m_readBytes = 1;   // Memory Access Control
    else if( m_rxReg == 0x37 ) m_readBytes = 1;   // Vertical Scrolling Start Address (2 params in datasheet??)
    //else if( m_rxReg == 0x38 ) // Idle Mode Off
    //else if( m_rxReg == 0x39 ) // Idle Mode On
    else if( m_rxReg == 0x3A ) m_readBytes = 1;   // COLMOD: Pixel Formay Set
    //else if( m_rxReg == 0x3C ) // Write Memory Continue
    //else if( m_rxReg == 0x3E ) // Read Memory Continue
    else if( m_rxReg == 0x44 ) m_readBytes = 2;   // Set Tear Scanline
    else if( m_rxReg == 0x45 ) m_readBytes = 2;   // Get Scanline
    else if( m_rxReg == 0x51 ) m_readBytes = 2;   // Write Display Brightness
    else if( m_rxReg == 0x52 ) m_readBytes = 2;   // Read Display Brightness
    else if( m_rxReg == 0x53 ) m_readBytes = 1;   // Write CTRL Display
    else if( m_rxReg == 0x54 ) m_readBytes = 2;   // Write CTRL Display
    else if( m_rxReg == 0x55 ) m_readBytes = 1;   // Write Content Adaptive Brightness Control
    else if( m_rxReg == 0x55 ) m_readBytes = 2;   // Read Content Adaptive Brightness Control
    else if( m_rxReg == 0x5E ) m_readBytes = 1;   // Write CABC Minimum Brightness
    else if( m_rxReg == 0x5F ) m_readBytes = 2;   // Read CABC Minimum Brightness

    else if( m_rxReg == 0xB0 ) m_readBytes = 1;   // RGB Interface Signal Control
    else if( m_rxReg == 0xB1 ) m_readBytes = 2;   // Frame Rate Control (In Normal Mode/Full Colors)
    else if( m_rxReg == 0xB2 ) m_readBytes = 2;   // Frame Rate Control (In Idle Mode/8 colors)
    else if( m_rxReg == 0xB3 ) m_readBytes = 2;   // Frame Rate control (In Partial Mode/Full Colors)
    else if( m_rxReg == 0xB4 ) m_readBytes = 1;   // Display Inversion Control
    else if( m_rxReg == 0xB5 ) m_readBytes = 4;   // Blanking Porch Control
    else if( m_rxReg == 0xB6 ) m_readBytes = 3;   // Display Function Control
    else if( m_rxReg == 0xB7 ) m_readBytes = 1;   // Entry Mode Set
    else if( m_rxReg == 0xB8 ) m_readBytes = 1;   // Backlight Control 1
    else if( m_rxReg == 0xB9 ) m_readBytes = 1;   // Backlight Control 2
    else if( m_rxReg == 0xBA ) m_readBytes = 1;   // Backlight Control 3
    else if( m_rxReg == 0xBB ) m_readBytes = 1;   // Backlight Control 4
    else if( m_rxReg == 0xBC ) m_readBytes = 1;   // Backlight Control 5
    else if( m_rxReg == 0xBE ) m_readBytes = 1;   // Backlight Control 7
    else if( m_rxReg == 0xBF ) m_readBytes = 1;   // Backlight Control 8
    else if( m_rxReg == 0xC0 ) m_readBytes = 1;   // Power Control 1
    else if( m_rxReg == 0xC1 ) m_readBytes = 1;   // Power Control 2
    else if( m_rxReg == 0xC5 ) m_readBytes = 2;   // VCOM Control 1
    else if( m_rxReg == 0xC7 ) m_readBytes = 1;   // VCOM Control 2
    else if( m_rxReg == 0xD0 ) m_readBytes = 2;   // NV Memory Write
    else if( m_rxReg == 0xD1 ) m_readBytes = 3;   // NV Memory Protection Key
    else if( m_rxReg == 0xD2 ) m_readBytes = 3;   // NV Memory Status Read
    else if( m_rxReg == 0xD3 ) m_readBytes = 4;   // Read ID4
    else if( m_rxReg == 0xDA ) m_readBytes = 2;   // Read ID1
    else if( m_rxReg == 0xDB ) m_readBytes = 2;   // Read ID2
    else if( m_rxReg == 0xDC ) m_readBytes = 2;   // Read ID3
    else if( m_rxReg == 0xE0 ) m_readBytes = 15;  // Positive Gamma Correction
    else if( m_rxReg == 0xE1 ) m_readBytes = 15;  // Negative Gamma Correction
    else if( m_rxReg == 0xE2 ) m_readBytes = 16;  // Digital Gamma Control 1
    else if( m_rxReg == 0xE3 ) m_readBytes = 64;  // Digital Gamma Control 2
    else if( m_rxReg == 0xF6 ) m_readBytes = 3;   // Interface Control

    // Misterious Commands in Adafruit_ILI9341.cpp initcmd[]
    else if( m_rxReg == 0xEF ) m_readBytes = 3; //, 0x03, 0x80, 0x02,
    else if( m_rxReg == 0xCF ) m_readBytes = 3; //, 0x00, 0xC1, 0x30,
    else if( m_rxReg == 0xED ) m_readBytes = 4; //, 0x64, 0x03, 0x12, 0x81,
    else if( m_rxReg == 0xE8 ) m_readBytes = 3; //, 0x85, 0x00, 0x78,
    else if( m_rxReg == 0xCB ) m_readBytes = 5; //, 0x39, 0x2C, 0x00, 0x34, 0x02,
    else if( m_rxReg == 0xF7 ) m_readBytes = 1; //, 0x20,
    else if( m_rxReg == 0xEA ) m_readBytes = 2; //, 0x00, 0x00,
    else if( m_rxReg == 0xF2 ) m_readBytes = 1; //, 0x00, 0x00,

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
