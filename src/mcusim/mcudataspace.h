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

#include "mcutypes.h"

class MAINMODULE_EXPORT DataSpace
{
    public:
        DataSpace();
        ~DataSpace();

        void initialize();

        uint8_t* getRam() { return m_dataMem.data(); }  // Get pointer to Ram data
        uint16_t getMapperAddr( uint16_t addr ) { return m_addrMap[addr]; } // Get mapped addresses in Data space

        uint8_t  readReg( uint16_t addr );            // Read Register (call watchers)
        void     writeReg(uint16_t addr, uint8_t v, bool masked=true);// Write Register (call watchers)

        uint8_t* getReg( QString reg )                // Get pointer to Reg data by name
        { return &m_dataMem[m_regInfo.value( reg ).address]; }
        bool regExist( QString reg )                  // Does a register with this name exist?
        { return m_regInfo.contains( reg ); }

        QHash<QString, uint8_t>*       bitMasks() { return &m_bitMasks; }
        QHash<QString, uint16_t>*      bitRegs() { return &m_bitRegs; }
        QHash<QString, regInfo_t>*     regInfo()  { return &m_regInfo; }
        QHash<uint16_t, regSignal_t*>* regSignals() { return &m_regSignals; }

        int m_regOverride;                         // Register value is overriden at write time

    protected:
        uint16_t m_regStart;                       // First address of SFR section
        uint16_t m_regEnd;                         // Last  address of SFR Section

        std::vector<uint16_t> m_addrMap;           // Maps addresses in Data space
        std::vector<uint8_t>  m_dataMem;           // Whole Ram space including Registers
        std::vector<uint8_t>  m_regMask;           // Registers Write mask

        QHash<QString, regInfo_t>     m_regInfo;   // Access Reg Info by  Reg name
        QHash<uint16_t, regSignal_t*> m_regSignals;// Access Reg Signals by Reg address
        QHash<QString, uint8_t>       m_bitMasks;  // Access Bit mask by bit name
        QHash<QString, uint16_t>      m_bitRegs;   // Access Reg. address by bit name

        uint16_t m_sregAddr;                       // STATUS Reg Address
};

#endif
