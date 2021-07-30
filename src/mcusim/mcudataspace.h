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

#ifndef MCUDATASPACE_H
#define MCUDATASPACE_H

#include <QHash>
#include <vector>
#include <QDebug>

#include "mcusignal.h"
#include "mcutypes.h"


class MAINMODULE_EXPORT DataSpace
{
    public:
        DataSpace();
        ~DataSpace();

        struct regInfo_t{
            uint16_t address;
            uint8_t  resetVal;
        };

        struct regSignal_t{
            McuSignal on_write;
            McuSignal on_read;
        };

        void initialize();

        uint8_t* getRam() { return m_dataMem.data(); }  // Get pointer to Ram data
        uint16_t getMapperAddr( uint16_t addr ) { return m_addrMap[addr]; } // Get mapped addresses in Data space

        uint8_t  readReg( uint16_t addr );            // Read Register (call watchers)
        void     writeReg( uint16_t addr, uint8_t v );// Write Register (call watchers)

        uint8_t* getReg( QString reg )                // Get pointer to Reg data by name
        { return &m_dataMem[m_regInfo.value( reg ).address]; }

        template <class T>                // Add callback for Register changes by names
        void watchRegNames( QString regNames, int write
                      , T* inst, void (T::*func)(uint8_t) )
        {
            if( regNames.isEmpty() ) return;

            QStringList regs = regNames.split(",");
            for( QString reg : regs )
            {
                uint16_t addr = m_regInfo.value( reg ).address;
                watchRegister( addr, write, inst, func, 0xFF );
            }
        }
        template <class T>                // Add callback for Register changes by address
        void watchRegister( uint16_t addr, int write
                          , T* inst, void (T::*func)(uint8_t), uint8_t mask=0xFF )
        {
            regSignal_t* regSignal = m_regSignals.value( addr );
            if( !regSignal )
            {
                regSignal = new regSignal_t;
                m_regSignals.insert( addr, regSignal );
            }
            if( write ) regSignal->on_write.connect( inst, func, mask );
            else        regSignal->on_read.connect( inst, func, mask  );
        }

        template <class T>              // Add callback for Register bit changes by names
        void watchBitNames( QString bitNames, int write
                      , T* inst, void (T::*func)(uint8_t) )
        {
            if( bitNames.isEmpty() ) return;

            uint16_t regAddr = 0;
            QStringList bitList = bitNames.split(",");
            uint8_t     bitMask = getBitMask( bitList );

            regAddr = m_bitRegs.value( bitList.first() );

            if( regAddr )
                watchRegister( regAddr, write, inst, func, bitMask );
        }
        uint8_t getBitMask( QStringList bitList ) // Get mask for a group of bits in a Register
        {
            uint8_t bitMask = 0;
            for( QString bitName : bitList ) bitMask |= m_bitMasks.value( bitName );
            return bitMask;
        }
        regBits_t getRegBits( QString bitNames ) // Get a set of consecutive bits in a Register
        {
            regBits_t regBits;
            QStringList bitList = bitNames.split(",");

            uint8_t mask = getBitMask( bitList );
            regBits.mask = mask;

            for( regBits.bit0=0; regBits.bit0<8; ++regBits.bit0 ) // Rotate mask to get initial bit
            {
                if( mask & 1 ) break;
                mask >>= 1;
            }
            uint16_t regAddr = m_bitRegs.value( bitList.first() );
            regBits.reg = &m_dataMem[regAddr];

            return regBits;
        }

        int m_regOverride;                         // Register value is overriden at write time

    protected:
        uint16_t m_regStart;                       // First address of SFR section
        uint16_t m_regEnd;                         // Last  address of SFR Section

        std::vector<uint16_t> m_addrMap;           // Maps addresses in Data space
        std::vector<uint8_t>  m_dataMem;           // Whole Ram space including Registers

        QHash<QString, regInfo_t>     m_regInfo;   // Access Reg Info by  Reg name
        QHash<uint16_t, regSignal_t*> m_regSignals;// Access Reg Signals by Reg address
        QHash<QString, uint8_t>       m_bitMasks;  // Access Bit mask by bit name
        QHash<QString, uint16_t>      m_bitRegs;   // Access Reg. address by bit name

        uint16_t m_sregAddr;                       // STATUS Reg Address
};

#endif
