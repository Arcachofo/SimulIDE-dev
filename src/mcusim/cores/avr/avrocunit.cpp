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

#include "avrocunit.h"
#include "mcupin.h"
#include "e_mcu.h"


AvrOcUnit::AvrOcUnit( eMcu* mcu, QString name )
         : McuOcUnit( mcu, name )
{
}

AvrOcUnit::~AvrOcUnit( )
{
}

/*void AvrOcUnit::initialize()
{
}*/

void AvrOcUnit::configure( uint8_t val ) // COMNX0,COMNX1
{
    uint8_t mode = getRegBitsVal( val, m_configBits );//((val & m_configMask)>>m_maskBit0);

    if(  mode != m_mode )
    {
        m_mode =  mode;

        if( m_mode == 0 )           // OC Pin disconnected
        {
             m_ocPin->controlPin( false, false );
        }
        else                        // OC Pin connected
        {
             m_ocPin->controlPin( true, true );
             m_ocPin->setOutState( false );
        }
    }
}

