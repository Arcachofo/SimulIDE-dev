/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "avrtimer.h"
#include "avrocunit.h"
#include "e_mcu.h"
#include "simulator.h"

McuTimer* AvrTimer::makeTimer( eMcu* mcu, QString name ) // Static
{
    QString n = name.toLower();
    if     ( n == "timer0" ) return new AvrTimer0( mcu, name );
    else if( n == "timer1" ) return new AvrTimer1( mcu, name );
    else if( n == "timer2" ) return new AvrTimer2( mcu, name );
    return NULL;
}

AvrTimer::AvrTimer(  eMcu* mcu, QString name )
        : McuTimer( mcu, name )
{
}
AvrTimer::~AvrTimer(){}

void AvrTimer::initialize()
{
    McuTimer::initialize();

    m_ovfMatch  = m_maxCount;
    m_ovfPeriod = m_ovfMatch + 1;

    m_wgmMode = wgmNORMAL;
    m_WGM10 = 0;
    m_WGM32 = 0;
}

void AvrTimer::addOcUnit( McuOcUnit* ocUnit )
{
    m_ocUnit.emplace_back( ocUnit );

    if     ( ocUnit->getId().endsWith("A") ) m_OCA = ocUnit;
    else if( ocUnit->getId().endsWith("B") ) m_OCB = ocUnit;
}

McuOcUnit* AvrTimer::getOcUnit( QString name )
{
    if     ( name.endsWith("A") ) return m_OCA;
    else if( name.endsWith("B") ) return m_OCB;
    return NULL;
}

void AvrTimer::configureA( uint8_t val ) // TCCRXA  // WGM00,WGM01
{
    m_OCA->configure( val & 0b11000000 );
    m_OCB->configure( val & 0b00110000 );

    m_WGM10 = val & 0b00000011;  // WGMX1,WGMX0
    updtWgm();
}

void AvrTimer::configureB( uint8_t val ) // TCCRXB
{
    uint8_t mode = val & 0b00000111; // CSX0-3

    /// Not working after Rev 376
    /*if( mode != m_mode )
    {
        m_mode = mode;

        updtCount();    // write counter values to Ram
        if( mode ) configureClock();
        updtCycles();  // This will shedule or cancel events
        //enable( m_mode );
        m_running = ( val > 0 );
    }*/
    if( mode != m_mode )
    {
        m_mode = mode;
        if( mode ) configureClock();
        enable( m_mode );
    }
    m_WGM32 = (val & 0b00011000)>>1; // WGMX3,WGMX2
    updtWgm();
}

void AvrTimer::configureClock()
{
    m_prescaler = m_prescList.at( m_mode );
    m_clkSrc = clkMCU;
}

void AvrTimer::configureExtClock()
{
    m_prescaler = 1;
    m_clkSrc = clkEXT;
    /// if     ( m_mode == 6 ) m_clkEdge = Clock_Falling;
    /// else if( m_mode == 7 ) m_clkEdge = Clock_Rising;
}

void AvrTimer::configureOcUnits( bool disable )
{
    m_bidirec = false;
    m_reverse = false;

    ocAct_t comActA = (ocAct_t)m_OCA->getMode(); // Default modes
    ocAct_t comActB = (ocAct_t)m_OCB->getMode();
    ocAct_t tovActA = ocNONE;
    ocAct_t tovActB = ocNONE;

    if( m_wgmMode == wgmPHASE )  // Phase Correct PWM
    {
        if((comActA == ocTOGGLE) && disable ) comActA = ocNONE;
        if( comActB == ocTOGGLE ) comActB = ocNONE;
        m_bidirec = true;
    }
    else  if( m_wgmMode == wgmFAST )  // Fast PWM
    {
        if((comActA == ocTOGGLE) && disable ) comActA = ocNONE;
        if     ( comActA == ocCLEAR ) tovActA = ocSET;
        else if( comActA == ocSET )   tovActA = ocCLEAR;

        if     ( comActB == ocTOGGLE ) comActB = ocNONE;
        else if( comActB == ocCLEAR )  tovActB = ocSET;
        else if( comActB == ocSET )    tovActB = ocCLEAR;
    }
    m_OCA->setOcActs( comActA, tovActA );
    m_OCB->setOcActs( comActB, tovActB );

    if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
    else            m_ovfPeriod = m_ovfMatch+1;
}

//--------------------------------------------------
// TIMER 8 Bit--------------------------------------

#define OCRXA8 m_ocrxaL[0]

AvrTimer8bit::AvrTimer8bit( eMcu* mcu, QString name )
            : AvrTimer( mcu, name )
{
    m_maxCount = 0xFF;
}
AvrTimer8bit::~AvrTimer8bit(){}

void AvrTimer8bit::updtWgm()
{
    m_wgmMode = (wgmMode_t)m_WGM10;
    configureOcUnits( !m_WGM32 );
    OCRXAchanged( OCRXA8 );
}

void AvrTimer8bit::OCRXAchanged( uint8_t val )
{
    m_ovfMatch = 0xFF;
    if( (m_wgmMode == wgmCTC)
      ||((m_WGM32) && (( m_wgmMode == wgmPHASE)
                      ||(m_wgmMode == wgmFAST)) ) )
    {
        m_ovfMatch = val;
    }
    if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
    else            m_ovfPeriod = m_ovfMatch+1;
}

void AvrTimer8bit::setOCRXA( QString reg )
{
    m_ocrxaL = m_mcu->getReg( reg );
    m_mcu->watchRegNames( reg, R_WRITE, this, &AvrTimer8bit::OCRXAchanged );
}

//--------------------------------------------------
// TIMER 0 -----------------------------------------

AvrTimer0::AvrTimer0( eMcu* mcu, QString name)
         : AvrTimer8bit( mcu, name )
{
    setOCRXA( "OCR0A" );
}
AvrTimer0::~AvrTimer0(){}

void AvrTimer0::configureClock()
{
    if( m_mode > 5 ) AvrTimer::configureExtClock();
    else             AvrTimer::configureClock();
}

//--------------------------------------------------
// TIMER 2 -----------------------------------------

AvrTimer2::AvrTimer2( eMcu* mcu, QString name)
         : AvrTimer8bit( mcu, name )
{
    setOCRXA( "OCR2A" );
}
AvrTimer2::~AvrTimer2(){}


//--------------------------------------------------
// TIMER 16 Bit-------------------------------------

#define OCRXA16 m_ocrxaL[0]+(m_ocrxaH[0]<<8)
#define ICRX16  m_icrxL[0]+(m_icrxH[0]<<8)

AvrTimer16bit::AvrTimer16bit( eMcu* mcu, QString name )
             : AvrTimer( mcu, name )
{
    m_maxCount = 0xFFFF;
}
AvrTimer16bit::~AvrTimer16bit(){}

void AvrTimer16bit::updtWgm()
{
    uint8_t WGM = m_WGM32 + m_WGM10;
    switch( WGM )
    {
        case 0: // Normal
            m_wgmMode = wgmNORMAL;
            m_ovfMatch = 0xFFFF;
            break;
        case 1: // PWM, Phase Correct, 8-bit
            m_wgmMode = wgmPHASE;
            m_ovfMatch = 0x00FF;
            break;
        case 2: // PWM, Phase Correct, 9-bit
            m_wgmMode = wgmPHASE;
            m_ovfMatch = 0x01FF;
            break;
        case 3: // PWM, Phase Correct, 10-bit
            m_wgmMode = wgmPHASE;
            m_ovfMatch = 0x03FF;
            break;
        case 4: // CTC
            m_wgmMode = wgmCTC;
            m_ovfMatch = OCRXA16;
            break;
        case 5: // Fast PWM, 8-bit
            m_wgmMode = wgmFAST;
            m_ovfMatch = 0x00FF;
            break;
        case 6: // Fast PWM, 9-bit
            m_wgmMode = wgmFAST;
            m_ovfMatch = 0x01FF;
            break;
        case 7: // Fast PWM, 10-bit
            m_wgmMode = wgmFAST;
            m_ovfMatch = 0x03FF;
            break;
        case 8: // PWM, Phase and Frequency Correct
            m_wgmMode = wgmPHASE;
            m_ovfMatch = ICRX16;
            break;
        case 9: // PWM, Phase and Frequency Correct
            m_wgmMode = wgmPHASE;
            m_ovfMatch = OCRXA16;
            break;
        case 10: // PWM, Phase Correct
            m_wgmMode = wgmPHASE;
            m_ovfMatch = ICRX16;
            break;
        case 11: // PWM, Phase Correct
            m_wgmMode = wgmPHASE;
            m_ovfMatch = OCRXA16;
            break;
        case 12: // CTC
            m_wgmMode = wgmCTC;
            m_ovfMatch = ICRX16;
            break;
        case 13: // (Reserved)
            m_wgmMode = wgmNORMAL;
            m_ovfMatch = 0xFFFF;
            break;
        case 14: // Fast PWM ICRX
            m_wgmMode = wgmFAST;
            m_ovfMatch = ICRX16;
            break;
        case 15: // Fast PWM OCRXA
            m_wgmMode = wgmFAST;
            m_ovfMatch = OCRXA16;
            break;
    }
    bool disable = (m_WGM32 & 0b00001000)==0;
    configureOcUnits( disable );
}

void AvrTimer16bit::OCRXAchanged( uint8_t val )
{
    updtWgm();
}

void AvrTimer16bit::setOCRXA( QString reg )
{
    QStringList list = reg.split(",");

    reg = list.takeFirst();
    m_ocrxaL = m_mcu->getReg( reg );
    m_mcu->watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged );

    reg = list.takeFirst();
    m_ocrxaH = m_mcu->getReg( reg );
    m_mcu->watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged );
}

void AvrTimer16bit::setICRX( QString reg )
{
    QStringList list = reg.split(",");

    reg = list.takeFirst();
    m_icrxL = m_mcu->getReg( reg );
    m_mcu->watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged );

    reg = list.takeFirst();
    m_icrxH = m_mcu->getReg( reg );
    m_mcu->watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged );
}

//--------------------------------------------------
// TIMER 1 -----------------------------------------

AvrTimer1::AvrTimer1( eMcu* mcu, QString name)
         : AvrTimer16bit( mcu, name )
{
    setOCRXA( "OCR1AL,OCR1AH" );
    setICRX( "ICR1L,ICR1H" );
}
AvrTimer1::~AvrTimer1(){}

void AvrTimer1::configureClock()
{
    if( m_mode > 5 ) AvrTimer::configureExtClock();
    else             AvrTimer::configureClock();
}


