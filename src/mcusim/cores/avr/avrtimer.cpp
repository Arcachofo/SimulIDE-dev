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

#define OCR0A m_ocr0a[0]

AvrTimer::AvrTimer()
{
}
AvrTimer::~AvrTimer(){}

McuTimer* AvrTimer::makeTimer( eMcu* mcu, QString name ) // Static
{
    if( name.toLower() == "timer0" ) return new AvrTimer0( mcu, name );
    return NULL;
}

// TIMER0 --------------------------------------

AvrTimer0::AvrTimer0( eMcu* mcu, QString name)
         : McuTimer( mcu, name )
{
    m_ocr0a = m_mcu->getReg( "OCR0A" );
    mcu->watchRegNames( "OCR0A", R_WRITE, this, &AvrTimer0::OCRAchanged );

}
AvrTimer0::~AvrTimer0(){}

void AvrTimer0::runEvent()
{
    McuTimer::runEvent();
}

void AvrTimer0::initialize()
{
    McuTimer::initialize();

    m_ovfMatch  = 0xFF;
    m_ovfPeriod = m_ovfMatch + 1;

    m_wgmMode = wgmNORMAL;
    m_WGM02 = false;
}

void AvrTimer0::addocUnit( McuOcUnit* ocUnit )
{
    m_ocUnit.emplace_back( ocUnit );

    if     ( ocUnit->getId().endsWith("A") ) m_OCA = ocUnit;
    else if( ocUnit->getId().endsWith("B") ) m_OCB = ocUnit;
}

void AvrTimer0::configureA( uint8_t val ) // TCCR0A  // WGM00,WGM01
{
    //val = getRegBitsVal( val, m_configBitsA );
    wgmMode_t wgmMode = (wgmMode_t)(val & 0b00000011);

    m_OCA->configure( val & 0b11000000 );
    m_OCB->configure( val & 0b00110000 );
    //bool  clkSrc = val & 0b00000100;

    //if( wgmMode != m_wgmMode )
    {
        m_wgmMode = wgmMode;
        updtWgm();
    }
}

void AvrTimer0::configureB( uint8_t val ) // TCCR0B
{
    //getRegBitsVal( val, m_configBitsB );
    uint8_t mode = val & 0b00000111; // CSX0-3

    if( mode != m_mode )
    {
        m_mode = mode;
        if( mode ) configureClock();
        enable( m_mode );
    }

    uint8_t wgm02 = val & 0b00001000; // WGM02
    if(  wgm02 != m_WGM02 )
    {
        m_WGM02 =  wgm02;
        updtWgm();
    }
}

void AvrTimer0::configureClock()
{
    m_clkSrc = clkMCU;

    if     ( m_mode == 1 ) m_prescaler = 1;
    else if( m_mode == 2 ) m_prescaler = 8;
    else if( m_mode == 3 ) m_prescaler = 64;
    else if( m_mode == 4 ) m_prescaler = 256;
    else if( m_mode == 5 ) m_prescaler = 1024;
    else
    {
        m_clkSrc = clkEXT;
        if     ( m_mode == 6 ) m_clkEdge = Clock_Falling;
        else if( m_mode == 7 ) m_clkEdge = Clock_Rising;
    }
}

void AvrTimer0::updtWgm()
{
    ocAct_t comActA = (ocAct_t)m_OCA->getMode();
    ocAct_t comActB = (ocAct_t)m_OCB->getMode();
    ocAct_t tovActA = ocNONE;
    ocAct_t tovActB = ocNONE;

    m_ovfMatch = 0;
    m_bidirec = false;
    m_reverse = false;

    if     ( m_wgmMode == wgmNORMAL )  // Normal
    {
        m_ovfMatch = 0xFF;
    }
    else if( m_wgmMode == wgmPHASE )  // Phase Correct PWM
    {
        if( m_WGM02 )
        {
            m_ovfMatch = OCR0A;
        }
        else {
            m_ovfMatch = 0xFF;
            if( comActA == ocTOGGLE ) comActA = ocNONE;
        }
        if( comActB == ocTOGGLE ) comActB = ocNONE;
        m_bidirec = true;
    }
    else if( m_wgmMode == wgmCTC )    // CTC
    {
        m_ovfMatch = OCR0A;
    }
    else  if( m_wgmMode == wgmFAST )  // Fast PWM
    {
        if( m_WGM02 )
        {
            m_ovfMatch = OCR0A;
            if( comActA == ocTOGGLE ) comActA = ocTOGGLE;
        }
        else {
            m_ovfMatch = 0xFF;
            if( comActA == ocTOGGLE ) comActA = ocNONE;
        }
        if     ( comActA == ocCLEAR ) tovActA = ocSET;
        else if( comActA == ocSET )   tovActA = ocCLEAR;

        if     ( comActB == ocTOGGLE ) comActB = ocNONE;
        else if( comActB == ocCLEAR )  tovActB = ocSET;
        else if( comActB == ocSET )    tovActB = ocCLEAR;
    }
    if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
    else            m_ovfPeriod = m_ovfMatch+1;

    m_OCA->setOcActs( comActA, tovActA );
    m_OCB->setOcActs( comActB, tovActB );
}

void AvrTimer0::OCRAchanged( uint8_t val )
{
    if( (m_wgmMode == wgmCTC)
      ||((m_WGM02) && (( m_wgmMode == wgmPHASE)
                      ||(m_wgmMode == wgmFAST)) ) )
    {
        m_ovfMatch = val;
        if( m_bidirec ) m_ovfPeriod = m_ovfMatch;
        else            m_ovfPeriod = m_ovfMatch+1;
    }
}
