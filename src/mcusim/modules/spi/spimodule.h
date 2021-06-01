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

#ifndef SPIMODULE_H
#define SPIMODULE_H

#include "e-clocked_device.h"

class IoPin;

class MAINMODULE_EXPORT SpiModule : public eClockedDevice
{
    public:
        SpiModule( QString name );
        ~SpiModule();

        enum spiMode_t{
            SPI_OFF=0,
            SPI_MASTER,
            SPI_SLAVE
        };

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void setMode( spiMode_t mode );

        virtual void readByte();
        virtual void writeByte();

        virtual void setMosiPin( IoPin* pin ) { m_MOSI = pin; }
        virtual void setMisoPin( IoPin* pin ) { m_MISO = pin; }
        virtual void setSckPin( IoPin* pin )  { m_clkPin = pin; }
        virtual void setSsPin( IoPin* pin )   { m_SS = pin; }

    protected:
        inline void readBit();
        inline void writeBit();

        uint64_t m_clockPeriod;   // SPI Clock half period in ps

        //bool m_dataState;
        bool m_lsbFirst;

        clkState_t m_sampleEdge;
        clkState_t m_leadEdge;

        int m_bitPtr;       // Bit Pointer
        int m_bitInc;       // Bit Pointer increment: LSB first or MSB first??

        uint8_t m_txReg;    // Byte to Send
        uint8_t m_rxReg;    // Byte Received

        spiMode_t m_mode;

        IoPin* m_MOSI;
        IoPin* m_MISO;
        //IoPin* m_SCK;
        IoPin* m_SS;

        IoPin* m_dataOut;
        IoPin* m_dataIn;
};
#endif

