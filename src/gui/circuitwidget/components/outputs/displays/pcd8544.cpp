/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
// Based on:
//
// C++ Implementation: pcd8544
//
// Description: This component emulates a graphic LCD module based on the
//              PCD8544 controller.
//
// Author: Roland Elek <elek.roland@gmail.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution

#include <QPainter>

#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "pcd8544.h"


Component* Pcd8544::construct( QObject* parent, QString type, QString id )
{ return new Pcd8544( parent, type, id ); }

LibraryItem* Pcd8544::libraryItem()
{
    return new LibraryItem(
        "Pcd8544",
        "Displays",
        "pcd8544.png",
        "Pcd8544",
        Pcd8544::construct );
}

Pcd8544::Pcd8544( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement(  id+"-eElement"  )
       , m_pRst( 270, QPoint(-32, 40), id+"-PinRst", 0, this )
       , m_pCs ( 270, QPoint(-16, 40), id+"-PinCs" , 0, this )
       , m_pDc ( 270, QPoint(  0, 40), id+"-PinDc" , 0, this )
       , m_pSi ( 270, QPoint( 16, 40), id+"-PinSi" , 0, this )
       , m_pScl( 270, QPoint( 32, 40), id+"-PinScl", 0, this )
{
    m_graphical = true;

    m_area = QRectF( -52, -52, 104, 84 );

    m_pin.resize( 5 );
    m_pin[0] = &m_pRst;
    m_pin[1] = &m_pCs;
    m_pin[2] = &m_pDc;
    m_pin[3] = &m_pSi;
    m_pin[4] = &m_pScl;
    
    m_pRst.setLabelText( "RST" );
    m_pCs.setLabelText(  "CS" );
    m_pDc.setLabelText(  "D/C" );
    m_pSi.setLabelText(  "DIN" );
    m_pScl.setLabelText( "CLK" );
    
    m_pdisplayImg = new QImage( 84, 48, QImage::Format_MonoLSB );
    m_pdisplayImg->setColor( 1, qRgb(0,0,0));
    m_pdisplayImg->setColor( 0, qRgb(200,215,180) );
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos( -32,-66, 0);
    setShowId( true );
    
   Pcd8544::initialize();
}
Pcd8544::~Pcd8544(){}

void Pcd8544::stamp()
{
    m_pScl.changeCallBack( this ); // Register for Scl changes callback
    m_pRst.changeCallBack( this ); // Register for Rst changes callback
    m_pCs.changeCallBack( this ); // Register for Cs changes callback
}

void Pcd8544::initialize()
{
    clearDDRAM();
    clearLcd();
    reset() ;
    Pcd8544::updateStep();
}

void Pcd8544::updateStep()
{
    if     ( m_bPD )          m_pdisplayImg->fill(0); // Power-Down mode
    else if( !m_bD && !m_bE ) m_pdisplayImg->fill(0); // Blank Display mode, blank the visuals
    else if( !m_bD && m_bE )  m_pdisplayImg->fill(1); //All segments on
    else
    {
        for(int row=0;row<6;row++){
            for( int col=0; col<84; col++ )
            {
                char abyte = m_aDispRam[row][col];
                for( int bit=0; bit<8; bit++ )
                {
                    //This takes inverse video mode into account:
                    m_pdisplayImg->setPixel(col,row*8+bit,
                        (abyte & 1) ^ ((m_bD && m_bE) ? 1 : 0) );

                    abyte >>= 1;
    }   }   }   }
    update();
}

void Pcd8544::voltChanged()               // Called when Scl, Rst or Cs Pin changes
{
    if( m_pRst.getVoltage()<0.3 )            // Reset Pin is Low
    {
        reset();
        return;
    }
    if( m_pCs.getVoltage()>1.6 )            // Cs Pin High: Lcd not selected
    {
        m_cinBuf = 0;                    // Initialize serial buffer
        m_inBit  = 0;
        return;
    }
    if( m_pScl.getVoltage()<1.6 )           // This is an Scl Falling Edge
    {
        m_lastScl = false;
        return;
    }
    else if( m_lastScl ) return;         // Not a rising edge
    m_lastScl = true;

    m_cinBuf &= ~1; //Clear bit 0
    
    if( m_pSi.getVoltage()>1.6 ) m_cinBuf |= 1;
    
    if( m_inBit == 7 ) 
    {
        if( m_pDc.getVoltage()>1.6 )                        // Write Data
        {
            //qDebug() << "Pcd8544::setVChanged"<< m_addrY<<m_addrX<< m_cinBuf;
            m_aDispRam[m_addrY][m_addrX] = m_cinBuf;
            incrementPointer();
        } 
        else{                                           // Write Command
            //qDebug() << "Pcd8544::setVChanged    Command:  "<< m_cinBuf;
            //if(m_cinBuf == 0) { //(NOP) } 
                
            if((m_cinBuf & 0xF8) == 0x20)               // Function set
            {
                m_bH  = ((m_cinBuf & 1) == 1);
                m_bV  = ((m_cinBuf & 2) == 2);
                m_bPD = ((m_cinBuf & 4) == 4);
            }else{
                if(m_bH) 
                {
                    //(Extended instruction set)
                    //None implemented yet - are they relevant at all?
                    //Visualization of e.g. contrast setting could be
                    //useful in some cases, meaningless in others.
                } 
                else                            // Basic instruction set 
                {
                    if((m_cinBuf & 0xFA) == 0x08)     // Display control
                    {
                        m_bD = ((m_cinBuf & 0x04) == 0x04);
                        m_bE =  (m_cinBuf & 0x01);
                    } 
                    else if((m_cinBuf & 0xF8) == 0x40)// Set Y RAM address
                    {
                        int addrY = m_cinBuf & 0x07;
                        if( addrY<6 ) m_addrY = addrY;
                    } 
                    else if((m_cinBuf & 0x80) == 0x80)// Set X RAM address
                    {
                        int addrX = m_cinBuf & 0x7F;
                        if( addrX<84 ) m_addrX = addrX;
        }   }   }   }
        m_inBit = 0;
    }else{
        m_cinBuf <<= 1;
        m_inBit++;
}   }

void Pcd8544::clearLcd() { m_pdisplayImg->fill(0); }

void Pcd8544::clearDDRAM() 
{
    for(int row=0; row<6; row++)
        for( int col=0; col<84; col++ )
            m_aDispRam[row][col] = 0;
}

void Pcd8544::incrementPointer() 
{
    if( m_bV )
    {
        m_addrY++;
        if( m_addrY >= 6 ) { m_addrY = 0; m_addrX++; }
        if( m_addrX >= 84 )  m_addrX = 0;
    }else{
        m_addrX++;
        if( m_addrX >= 84 ) { m_addrX = 0; m_addrY++; }
        if( m_addrY >= 6  )   m_addrY = 0;
}   }

void Pcd8544::reset() 
{
    m_cinBuf = 0;
    m_inBit  = 0;
    m_addrX  = 0;
    m_addrY  = 0;
    m_bPD = true;
    m_bV  = false;
    m_bH  = false;
    m_bE  = false;
    m_bD  = false;
}

void Pcd8544::remove()
{
    delete m_pdisplayImg;
    Component::remove();
}

void Pcd8544::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );
    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( -48, -48, 96, 60, 8, 8 );
    p->drawImage(-42,-42,*m_pdisplayImg );
}
