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

#include "avrspi.h"
//#include "mcupin.h"
#include "e_mcu.h"

AvrSpi::AvrSpi( eMcu* mcu, QString name )
      : McuSpi( mcu, name )
{
}
AvrSpi::~AvrSpi(){}

void AvrSpi::initialize()
{
    McuSpi::initialize();
}

void AvrSpi::configureA( uint8_t newSPCR ) // SPCR is being written
{

}

void AvrSpi::writeStatus( uint8_t newSPSR ) // SPSR is being written
{
    m_mcu->m_regOverride = newSPSR | (*m_spiStatus & 0b00000001); // Preserve Status bits
}

void AvrSpi::writeSpiReg( uint8_t newSPDR ) // SPDR is being written
{

}
