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
#include "datautils.h"
#include "avrocunit.h"
#include "e_mcu.h"
#include "simulator.h"
#include "regwatcher.h"

McuTimer* AvrTimer::createTimer( eMcu* mcu, QString name, int type ) // Static
{
    if     ( type == 80 )  return new AvrTimer80( mcu, name );
    else if( type == 81 )  return new AvrTimer81( mcu, name );
    else if( type == 82 )  return new AvrTimer82( mcu, name );
    else if( type == 160 ) return new AvrTimer16bit( mcu, name );

    return NULL;
}

AvrTimer::AvrTimer(  eMcu* mcu, QString name )
        : McuTimer( mcu, name )
{
    m_OCA = NULL;
    m_OCB = NULL;
    m_OCC = NULL;
}
AvrTimer::~AvrTimer(){}

void AvrTimer::initialize()
{
    McuTimer::initialize();

    m_ovfMatch  = m_maxCount;
    m_ovfPeriod = m_ovfMatch + 1;

    m_wgmMode = wgmNORM;
    m_WGM10 = 0;
    m_WGM32 = 0;
}

void AvrTimer::addOcUnit( McuOcUnit* ocUnit )
{
    m_ocUnit.emplace_back( ocUnit );

    if     ( ocUnit->getId().endsWith("A") ) m_OCA = ocUnit;
    else if( ocUnit->getId().endsWith("B") ) m_OCB = ocUnit;
    else if( ocUnit->getId().endsWith("C") ) m_OCC = ocUnit;
}

McuOcUnit* AvrTimer::getOcUnit( QString name )
{
    if     ( name.endsWith("A") ) return m_OCA;
    else if( name.endsWith("B") ) return m_OCB;
    else if( name.endsWith("C") ) return m_OCC;
    return NULL;
}

void AvrTimer::configureA( uint8_t newTCCRXA ) // TCCRXA  // WGM00,WGM01
{
    if( m_OCA ) m_OCA->configure( newTCCRXA ); // Done in ocunits
    if( m_OCB ) m_OCB->configure( newTCCRXA );
    if( m_OCC ) m_OCC->configure( newTCCRXA );

    m_WGM10 = newTCCRXA & 0b00000011;  // WGMX1,WGMX0
    updtWgm();
}

void AvrTimer::configureB( uint8_t newTCCRXB ) // TCCRXB
{
    uint8_t prIndex = getRegBitsVal( newTCCRXB, m_prSelBits ); // CSX0-n

    if( prIndex != m_prIndex )
    {
        m_prIndex = prIndex;
        if( prIndex ) configureClock();
        enable( m_prIndex );
    }
    m_WGM32 = ( newTCCRXB & 0b00011000)>>1; // WGMX3,WGMX2
    updtWgm();
}

void AvrTimer::configureClock()
{
    m_prescaler = m_prescList.at( m_prIndex );
    m_clkSrc = clkMCU;
}

void AvrTimer::configureExtClock()
{
    m_prescaler = 1;
    m_clkSrc = clkEXT;
    /// if     ( m_prIndex == 6 ) m_clkEdge = Clock_Falling;
    /// else if( m_prIndex == 7 ) m_clkEdge = Clock_Rising;
}

void AvrTimer::configureOcUnits( bool wgm3 )
{
    m_bidirec = false;
    //m_reverse = false;

    ocAct_t comActA, comActB, comActC;
    ocAct_t tovActA = ocNON;
    ocAct_t tovActB = ocNON;
    ocAct_t tovActC = ocNON;

    if( m_OCA ) comActA = (ocAct_t)m_OCA->getMode(); // Default modes
    if( m_OCB ) comActB = (ocAct_t)m_OCB->getMode();
    if( m_OCC ) comActC = (ocAct_t)m_OCC->getMode();

    if( m_wgmMode == wgmPHAS )  // Phase Correct PWM
    {
        if( m_OCA ) { if((comActA == ocTOG) && wgm3 ) comActA = ocNON; }
        if( m_OCB ) { if( comActB == ocTOG ) comActB = ocNON; }
        if( m_OCC ) { if( comActC == ocTOG ) comActC = ocNON; }
        m_bidirec = true;
    }
    else  if( m_wgmMode == wgmFAST )  // Fast PWM
    {
        if( m_OCA ) {
            if((comActA == ocTOG) && wgm3 ) comActA = ocNON;
            if     ( comActA == ocCLR ) tovActA = ocSET;
            else if( comActA == ocSET ) tovActA = ocCLR;
        }
        if( m_OCB ) {
            if     ( comActB == ocTOG ) comActB = ocNON;
            else if( comActB == ocCLR ) tovActB = ocSET;
            else if( comActB == ocSET ) tovActB = ocCLR;
        }
        if( m_OCC ) {
            if     ( comActC == ocTOG ) comActC = ocNON;
            else if( comActC == ocCLR ) tovActC = ocSET;
            else if( comActC == ocSET ) tovActC = ocCLR;
    }   }
    if( m_OCA ) m_OCA->setOcActs( comActA, tovActA );
    if( m_OCB ) m_OCB->setOcActs( comActB, tovActB );
    if( m_OCC ) m_OCC->setOcActs( comActC, tovActC );

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
    if( *m_ocrxaL == val ) return;

    uint16_t  ovf = 0xFF;
    if( (m_wgmMode == wgmCTC)
      ||((m_WGM32) && ((m_wgmMode == wgmPHAS)
                     ||(m_wgmMode == wgmFAST)) ) )
    { ovf = val; } // Top = OCRA

    if( m_ovfMatch == ovf ) return;
    m_ovfMatch = ovf;

    if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
    else            m_ovfPeriod = m_ovfMatch+1;

    sheduleEvents();
}

void AvrTimer8bit::setOCRXA( QString reg )
{
    m_ocrxaL = m_mcu->getReg( reg );
    watchRegNames( reg, R_WRITE, this, &AvrTimer8bit::OCRXAchanged, m_mcu );
}

//--------------------------------------------------
// TIMER 0 -----------------------------------------

AvrTimer80::AvrTimer80( eMcu* mcu, QString name)
          : AvrTimer8bit( mcu, name )
{
    setOCRXA( "OCR0A" );
}
AvrTimer80::~AvrTimer80(){}

void AvrTimer80::configureClock()
{
    if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    else                AvrTimer::configureClock();
}

//--------------------------------------------------
// TIMER 1 (8 bits) --------------------------------

AvrTimer81::AvrTimer81( eMcu* mcu, QString name)
          : AvrTimer8bit( mcu, name )
{
    //setOCRXA( "OCR0A" );
}
AvrTimer81::~AvrTimer81(){}

void AvrTimer81::configureA( uint8_t newGTCCR ) // GTCCR
{
    /// if( m_OCA ) m_OCA->configure( newGTCCR ); // Done in ocunits
    //if( m_OCB ) m_OCB->configure( newGTCCR );
    //if( m_OCC ) m_OCC->configure( newGTCCR );

    //m_WGM10 = val & 0b00000011;  // WGMX1,WGMX0
    //updtWgm();
}

void AvrTimer81::configureB( uint8_t newTCCR1 ) // TCCR1
{
    uint8_t prIndex = getRegBitsVal( newTCCR1, m_prSelBits ); // CSX0-n

    if( prIndex != m_prIndex )
    {
        m_prIndex = prIndex;
        if( prIndex ) configureClock();
        enable( m_prIndex );
    }
    //m_WGM32 = ( newTCCR1 & 0b00011000)>>1; // WGMX3,WGMX2
    //updtWgm();
}

void AvrTimer81::configureClock()
{
    //if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    //else                AvrTimer::configureClock();
}


//--------------------------------------------------
// TIMER 2 -----------------------------------------

AvrTimer82::AvrTimer82( eMcu* mcu, QString name)
          : AvrTimer8bit( mcu, name )
{
    setOCRXA( "OCR2A" );
}
AvrTimer82::~AvrTimer82(){}


//--------------------------------------------------
// TIMER 16 Bit-------------------------------------

#define OCRXA16 *m_ocrxaL+(*m_ocrxaH<<8)
#define ICRX16  *m_icrxL+(*m_icrxH<<8)

AvrTimer16bit::AvrTimer16bit( eMcu* mcu, QString name )
             : AvrTimer( mcu, name )
{
    m_maxCount = 0xFFFF;

    QString num = name.right(1);
    setOCRXA( "OCR"+num+"AL,OCR"+num+"AH" );
    setICRX( "ICR"+num+"L,ICR"+num+"H" );
}
AvrTimer16bit::~AvrTimer16bit(){}

void AvrTimer16bit::updtWgm()
{
    uint8_t WGM = m_WGM32 + m_WGM10;

    wgmMode_t mode = wgmNORM;
    uint16_t  ovf  = 0xFFFF;

    switch( WGM ){
        case 1:  mode = wgmPHAS; ovf = 0x00FF;  break; // PWM, Phase Correct, 8-bit
        case 2:  mode = wgmPHAS; ovf = 0x01FF;  break; // PWM, Phase Correct, 9-bit
        case 3:  mode = wgmPHAS; ovf = 0x03FF;  break; // PWM, Phase Correct, 10-bit
        case 4:  mode = wgmCTC;  ovf = OCRXA16; break; // CTC
        case 5:  mode = wgmFAST; ovf = 0x00FF;  break; // Fast PWM, 8-bit
        case 6:  mode = wgmFAST; ovf = 0x01FF;  break; // Fast PWM, 9-bit
        case 7:  mode = wgmFAST; ovf = 0x03FF;  break; // Fast PWM, 10-bit
        case 8:  mode = wgmPHAS; ovf = ICRX16;  break; // PWM, Phase and Frequency Correct
        case 9:  mode = wgmPHAS; ovf = OCRXA16; break; // PWM, Phase and Frequency Correct
        case 10: mode = wgmPHAS; ovf = ICRX16;  break; // PWM, Phase Correct
        case 11: mode = wgmPHAS; ovf = OCRXA16; break; // PWM, Phase Correct
        case 12: mode = wgmCTC;  ovf = ICRX16;  break; // CTC
        case 13:                                break; // (Reserved)
        case 14: mode = wgmFAST; ovf = ICRX16;  break; // Fast PWM ICRX
        case 15: mode = wgmFAST; ovf = OCRXA16; break; // Fast PWM OCRXA
    }
    m_wgmMode = mode;
    bool shedule = m_ovfMatch != ovf;
    m_ovfMatch = ovf;
    bool wgm3 = (m_WGM32 & 1<<3)==0;
    configureOcUnits( wgm3 );
    if( shedule ) sheduleEvents();
}

void AvrTimer16bit::setOCRXA( QString reg )
{
    QStringList list = reg.split(",");

    reg = list.takeFirst();
    m_ocrxaL = m_mcu->getReg( reg );
    watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged, m_mcu );

    reg = list.takeFirst();
    m_ocrxaH = m_mcu->getReg( reg );
    watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged, m_mcu );
}

void AvrTimer16bit::setICRX( QString reg )
{
    QStringList list = reg.split(",");

    reg = list.takeFirst();
    m_icrxL = m_mcu->getReg( reg );
    watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged, m_mcu );

    reg = list.takeFirst();
    m_icrxH = m_mcu->getReg( reg );
    /// Low byte triggers red/write operations
    /// watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::OCRXAchanged, m_mcu );
}

void AvrTimer16bit::configureClock()
{
    if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    else                AvrTimer::configureClock();
}
