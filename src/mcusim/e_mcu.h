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

#ifndef EMCU_H
#define EMCU_H

#include <QHash>

#include "e-element.h"
#include "itemlibrary.h"
#include "regsignal.h"

class McuCore;
class McuPort;
class McuTimer;
class Interrupt;

enum{
    R_READ = 0,
    R_WRITE,
};

class MAINMODULE_EXPORT eMcu : public eElement
{
        friend class McuCreator;
        friend class McuCore;

    public:
        eMcu( QString id );
        ~eMcu();

        struct regInfo_t{
            uint16_t address;
            uint8_t  resetVal;
        };

        struct regSignal_t{
            RegSignal<uint8_t> on_write;
            RegSignal<uint8_t> on_read;
        };

        virtual void initialize() override;
        virtual void runEvent() override;

        uint8_t* getRam() { return m_dataMem.data(); }    // Get pointer to Ram data

        uint8_t  readReg( uint16_t addr );            // Read Register (call watchers)
        void     writeReg( uint16_t addr, uint8_t v );// Write Register (call watchers)

        uint8_t* getReg( QString reg )                // Get pointer to Reg data by name
        { return &m_dataMem[m_regInfo.value( reg ).address]; }

        int getRegAddress( QString reg )             // Get Reg address by name
        { return m_regInfo.value( reg ).address; }

        void readStatus( uint8_t v );                // Update STATUS Reg when is readen
        void writeStatus( uint8_t v );               // Update STATUS Reg when is written

        uint32_t ramSize() { return m_dataMem.size(); }
        uint32_t flashSize() { return m_progMem.size(); }

        void enableInterrupts( uint8_t en );

        template <typename T>                      // Add callback for Register changes by names
        void watchRegsName( QString regNames, int write
                      , T* inst, void (T::*func)(uint8_t) )
        {
            if( regNames.isEmpty() ) return;
            if( regNames.contains(","))
            {
                QStringList regs = regNames.split(",");
                for( QString reg : regs )
                {
                    uint16_t addr = m_regInfo.value( reg ).address;
                    watchRegister( addr, write, inst, func, 0xFF );
                }
            }
            else watchRegister( m_regInfo.value( regNames ).address, write, inst, func, 0xFF );
        }

        template <typename T>                       // Add callback for Register changes by address
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

        template <typename T>                      // Add callback for Register bit changes by names
        void watchBits( QString bitNames, int write
                      , T* inst, void (T::*func)(uint8_t) )
        {
            if( bitNames.isEmpty() ) return;

            uint8_t  bitMask = 0;
            if( bitNames.contains(","))
            {
                QStringList bits = bitNames.split(",");
                for( QString bit : bits ) bitMask |= m_bitMasks.value( bit );
            }
            else bitMask = m_bitMasks.value( bitNames );

            uint16_t regAddr = m_bitRegs.value( bitNames );
            watchRegister( regAddr, write, inst, func, bitMask );
        }

        McuCore* cpu;
        int cyclesDone;

    protected:
        std::vector<uint16_t> m_progMem;             // Program memory
        uint32_t m_progMemSize;
        uint8_t  m_wordSize;                       // Size of flash word in bytes

        std::vector<uint8_t> m_dataMem;                // Whole Ram space including Registers
        uint32_t m_dataMemSize;

        QHash<QString, regInfo_t>     m_regInfo;   // Access Reg Info by  Reg name
        QHash<uint16_t, regSignal_t*> m_regSignals;// Access Reg Signals by Reg address
        uint16_t m_regStart;                       // Start address of SFR section
        uint16_t m_regEnd;                         // Last address of SFR Section
        QHash<QString, uint8_t>  m_bitMasks;       // Register bits Register Mask
        QHash<QString, uint16_t> m_bitRegs;        // Register bits Register address

        std::vector<uint8_t> m_sreg;               // STATUS Reg splitted in bits
        uint16_t m_sregAddr;                       // STATUS Reg Address
        
        double m_freq;                             // Clock Frequency in MegaHerzs
        double m_instCycle;                        // Clock ticks per Instruction Cycle
};


#endif
