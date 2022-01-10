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
    if     ( type == 800 ) return new AvrTimer800( mcu, name );
    else if( type == 801 ) return new AvrTimer801( mcu, name );
    else if( type == 810 ) return new AvrTimer810( mcu, name );
    else if( type == 820 ) return new AvrTimer820( mcu, name );
    else if( type == 821 ) return new AvrTimer821( mcu, name );
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
    OCRXAchanged( *m_ocrxaL );
}

void AvrTimer8bit::OCRXAchanged( uint8_t val )
{
    if( *m_ocrxaL == val ) return;
    *m_ocrxaL = val;

    uint16_t  ovf = 0xFF;
    if( (m_wgmMode == wgmCTC)
      ||((m_WGM32) && ( (m_wgmMode == wgmPHAS)
                      ||(m_wgmMode == wgmFAST)) ) )
    { ovf = val; } // Top = OCRA

    if( m_ovfMatch != ovf ){
        m_ovfMatch = ovf;

        if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
        else            m_ovfPeriod = m_ovfMatch+1;

        sheduleEvents();
    }
    m_OCA->ocrWriteL( val );
}

//--------------------------------------------------
// TIMER 0 -----------------------------------------

AvrTimer800::AvrTimer800( eMcu* mcu, QString name)
          : AvrTimer8bit( mcu, name )
{
}
AvrTimer800::~AvrTimer800(){}

void AvrTimer800::configureClock()
{
    if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    else                AvrTimer::configureClock();
}

//--------------------------------------------------
// TIMER 0 -----------------------------------------

AvrTimer801::AvrTimer801( eMcu* mcu, QString name)
           : McuTimer( mcu, name )
{
    m_maxCount = 0xFF;
}
AvrTimer801::~AvrTimer801(){}

void AvrTimer801::initialize()
{
    McuTimer::initialize();

    m_ovfMatch  = m_maxCount;
    m_ovfPeriod = m_ovfMatch + 1;
}

void AvrTimer801::configureA( uint8_t newTCCR0 )
{
    uint8_t prIndex = getRegBitsVal( newTCCR0, m_prSelBits ); // CSX0-n

    if( prIndex != m_prIndex )
    {
        m_prIndex = prIndex;
        if( prIndex ) configureClock();
        enable( m_prIndex );
    }
}

void AvrTimer801::configureClock()
{
    if( m_prIndex > 5 ) //AvrTimer::configureExtClock();
    {
        m_prescaler = 1;
        m_clkSrc = clkEXT;
        /// if     ( m_prIndex == 6 ) m_clkEdge = Clock_Falling;
        /// else if( m_prIndex == 7 ) m_clkEdge = Clock_Rising;
    }
    else{               //AvrTimer::configureClock();
        m_prescaler = m_prescList.at( m_prIndex );
        m_clkSrc = clkMCU;
    }
}

//--------------------------------------------------
// TIMER 1 (8 bits) --------------------------------

AvrTimer810::AvrTimer810( eMcu* mcu, QString name)
           : AvrTimer8bit( mcu, name )
{
}
AvrTimer810::~AvrTimer810(){}

void AvrTimer810::configureA( uint8_t newGTCCR ) // GTCCR
{
    /// if( m_OCA ) m_OCA->configure( newGTCCR ); // Done in ocunits
    //if( m_OCB ) m_OCB->configure( newGTCCR );
    //if( m_OCC ) m_OCC->configure( newGTCCR );

    //m_WGM10 = val & 0b00000011;  // WGMX1,WGMX0
    //updtWgm();
}

void AvrTimer810::configureB( uint8_t newTCCR1 ) // TCCR1
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

void AvrTimer810::configureClock()
{
    //if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    //else                AvrTimer::configureClock();
}

//--------------------------------------------------
// TIMER 2 -----------------------------------------

AvrTimer820::AvrTimer820( eMcu* mcu, QString name)
           : AvrTimer8bit( mcu, name )
{
}
AvrTimer820::~AvrTimer820(){}

//--------------------------------------------------
// TIMER 2 -----------------------------------------

AvrTimer821::AvrTimer821( eMcu* mcu, QString name)
           : AvrTimer8bit( mcu, name )
{
}
AvrTimer821::~AvrTimer821(){}

void AvrTimer821::configureA( uint8_t newTCCR2 )
{
    if( m_OCA ) m_OCA->configure( newTCCR2 ); // COM20 COM21 Done in ocunits

    uint8_t prIndex = getRegBitsVal( newTCCR2, m_prSelBits ); // CSX0-n
    if( prIndex != m_prIndex )
    {
        m_prIndex = prIndex;
        if( prIndex ) configureClock();
        enable( m_prIndex );
    }
    uint8_t WGM10 = ((( newTCCR2 & 1<<6) >> 6)
                   | (( newTCCR2 & 1<<3) >> 2)); // WGM20 WGM21
    if( m_WGM10 != WGM10 ){
        m_WGM10 = WGM10;
        updtWgm();
    }
}

//--------------------------------------------------
// TIMER 16 Bit-------------------------------------

#define OCRXA16 *m_ocrxaL+(*m_ocrxaH<<8)
#define ICRX16  *m_icrxL+(*m_icrxH<<8)

AvrTimer16bit::AvrTimer16bit( eMcu* mcu, QString name )
             : AvrTimer( mcu, name )
{
    m_maxCount = 0xFFFF;

    QString num = name.right(1);
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

void AvrTimer16bit::OCRXAchanged( uint8_t val )
{
    if( *m_ocrxaL == val ) return;
    *m_ocrxaL = val;
    updtWgm();
    m_OCA->ocrWriteL( val );
}

void AvrTimer16bit::ICRXLchanged( uint8_t val )
{
    if( *m_icrxL == val ) return;
    *m_icrxL = val;
    updtWgm();
}

/*void AvrTimer16bit::ICRXHchanged( uint8_t val )
{
    if( *m_icrxH == val ) return;
    *m_icrxH = val;
    updtWgm();
}*/

void AvrTimer16bit::setICRX( QString reg )
{
    QStringList list = reg.split(",");

    reg = list.takeFirst();
    m_icrxL = m_mcu->getReg( reg );
    watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::ICRXLchanged, m_mcu );

    reg = list.takeFirst();
    m_icrxH = m_mcu->getReg( reg );
    /// Low byte triggers red/write operations
    /// watchRegNames( reg, R_WRITE, this, &AvrTimer16bit::ICRXLchanged, m_mcu );
}

void AvrTimer16bit::configureClock()
{
    if( m_prIndex > 5 ) AvrTimer::configureExtClock();
    else                AvrTimer::configureClock();
}
