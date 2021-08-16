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

#include "e-element.h"
#include "itemlibrary.h"
#include "mcuport.h"
#include "mcutimer.h"
#include "mcuuart.h"
#include "mcuinterrupts.h"
#include "mcudataspace.h"
#include "mcuinterface.h"

class McuCore;
class McuTimer;

enum{
    R_READ = 0,
    R_WRITE,
};

class MAINMODULE_EXPORT eMcu : public McuInterface, public DataSpace
{
        friend class McuCreator;
        friend class McuCore;
        friend class Mcu;

    public:
        eMcu( QString id );
        ~eMcu();

        virtual void initialize() override;
        virtual void runEvent() override;

        // Overrides of McuInterface-----------------------------------
        //-------------------------------------------------------------
        virtual void stepCpu() override;

        virtual uint8_t  getRamValue( int address ) override { return m_dataMem[address]; }
        virtual void     setRamValue( int address, uint8_t value ) override { m_dataMem[address] = value; }
        virtual uint16_t getFlashValue( int address ) override { return m_progMem[address]; }
        virtual void     setFlashValue( int address, uint16_t value ) override { m_progMem[address] = value; }
        virtual uint8_t  getRomValue( int address ) override { return m_eeprom[address]; }
        virtual void     setRomValue( int address, uint8_t value ) override { m_eeprom[address] = value; }

        virtual uint16_t getRegAddress( QString reg ) override  // Get Reg address by name
        { return m_regInfo.value( reg ).address; }

        virtual int status() override;
        virtual int pc() override;
        virtual uint64_t cycle() override { return m_cycle; }

        virtual void uartOut( int number, uint32_t value ) override;
        virtual void uartIn( int number, uint32_t value ) override;
        //-------------------------------------------------------------
        // End Overrides of McuInterface-------------------------------

        double freqMHz() { return m_freq; }
        void setFreq( double freq );
        uint64_t simCycPI() { return m_simCycPI; }  // Simulation cycles per instruction cycle
        //double cpi() { return m_cPerInst; }       // Clock ticks per Instruction Cycle

        McuPin* getPin( QString name ) { return m_ports.getPin( name ); }
        QHash<QString, McuPort*> getPorts() { return m_ports.getPorts(); }

        McuTimer* getTimer( QString name ) { return m_timers.getTimer( name ); }

        void enableInterrupts( uint8_t en ) { m_interrupts.enableGlobal( en ); }

        McuCore* cpu;
        int cyclesDone;

    protected:
        uint64_t m_cycle;
        std::vector<uint16_t> m_progMem;           // Program memory

        Interrupts m_interrupts;
        McuPorts   m_ports;
        McuTimers  m_timers;
        std::vector<McuModule*> m_modules;
        //McuUsarts  m_usarts;
        std::vector<McuUsart*> m_usarts;

        double m_freq;                             // Clock Frequency in MegaHerzs
        double m_cPerInst;                         // Clock ticks per Instruction Cycle
        uint64_t m_simCycPI;                       // Simulation cycles per Instruction Cycle
};


#endif
