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

#ifndef MCUSPI_H
#define MCUSPI_H

#include "mcumodule.h"
#include "spimodule.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuSpi : public McuModule, public SpiModule
{
        friend class McuCreator;

    public:
        McuSpi( eMcu* mcu, QString name );
        ~McuSpi();

        virtual void initialize() override;

        virtual void writeStatus( uint8_t val ){;}
        virtual void writeSpiReg( uint8_t val ){;}

    protected:

        uint8_t m_prescaler;
        std::vector<uint16_t> m_prescList; // Prescaler values

        uint8_t* m_dataReg;
        uint8_t* m_statReg;
};

#endif
