/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "picocunit.h"
#include "datautils.h"
#include "mcupin.h"
#include "e_mcu.h"

PicOcUnit::PicOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
}
PicOcUnit::~PicOcUnit( ){}

void PicOcUnit::configure( uint8_t val ) // CCPxM0,CCPxM1,CCPxM2,CCPxM3
{
    uint8_t mode = getRegBitsVal( val, m_configBits );
    if( (mode & 0b1100) != 0b1000 ) return; // No Compare Mode
    mode = mode & 0b11;

    if(  mode == m_mode ) return;
    m_mode =  mode;
    m_comAct = ocNONE;

    switch( mode ) {
        case 0: m_comAct = ocSET;      break; // Set OC Pin
        case 1: m_comAct = ocCLEAR;    break; // Clear OC Pin
        case 2: m_interrupt->raise();  break; // Software interrupt
        case 3:                         /// TODO special event trigger ADC
        {
            m_timer->resetTimer();
        }
    }
    m_ocPin->controlPin( m_comAct != ocNONE, false );
    if( m_comAct != ocNONE ) m_ocPin->setOutState( false );
}
