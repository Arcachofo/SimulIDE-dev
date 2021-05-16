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

#include "spimodule.h"
#include "iopin.h"
//#include "simulator.h"

SpiModule::SpiModule()
         : eClockedDevice()
{
    m_MOSI = NULL;
    m_MISO = NULL;
    m_SCK  = NULL;
    m_SS   = NULL;
}
SpiModule::~SpiModule( ){}


void SpiModule::setMosiPin( IoPin* pin )
{
    m_MOSI = pin;
}

void SpiModule::setMisoPin( IoPin* pin )
{
    m_MISO = pin;
}

void SpiModule::setSckPin( IoPin* pin )
{
    m_SCK = pin;
    m_clockPin = pin;
}

void SpiModule::setSsPin( IoPin* pin )
{
    m_SS = pin;
}
