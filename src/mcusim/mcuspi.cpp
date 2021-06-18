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

#include "mcuspi.h"
#include "iopin.h"

McuSpi::McuSpi( eMcu* mcu, QString name )
      : McuModule( mcu, name )
      , SpiModule( name )
{
}

McuSpi::~McuSpi()
{
}

void McuSpi::initialize()
{
    SpiModule::initialize();

}

void McuSpi::setMode( spiMode_t mode )
{
    /*switch (mode) {
    case SPI_OFF:
        {
            m_MOSI->controlPin( false );
            m_MISO->controlPin( false );
            m_clkPin->controlPin( false );
            m_SS->controlPin( false );
        }
        break;
    case SPI_MASTER:
            m_MISO->overrideDir( false ); //MISO as input, rest as user set
        break;
    case SPI_SLAVE:
            m_MOSI->overrideDir( false );     // MOSI input
            m_clockPin->overrideDir( false ); // SCK input
            m_SS->overrideDir( false );       // SS input
        break;
    }*/
}
