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
#include "mcutimer.h"
#include "mcuuart.h"
#include "mcuinterrupts.h"
#include "mcudataspace.h"

class CpuBase;
class McuTimer;
class McuWdt;
class McuSleep;
class McuCtrlPort;

enum{
    R_READ = 0,
    R_WRITE,
};

enum mcuState_t{
    mcuStopped=0,
    mcuRunning,
    mcuSleeping
};

class Mcu;
class IoPort;
class McuPort;
class McuVref;
class Component;
class ExtMemModule;

class MAINMODULE_EXPORT eMcu : public DataSpace, public eElement
{
        friend class McuCreator;
        friend class McuCpu;
        friend class Mcu;

    public:
        eMcu( Component* comp, QString id );
        ~eMcu();

 static eMcu* self() { return m_pSelf; }

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        void stepCpu();

        void setDebugger( BaseDebugger* deb );
        void setDebugging( bool d ) { m_debugging = d; }

        uint16_t getFlashValue( int address ) { return m_progMem[address]; }
        void     setFlashValue( int address, uint16_t value ) { m_progMem[address] = value; }
        uint32_t flashSize(){ return m_flashSize; }
        uint32_t wordSize() { return m_wordSize; }

        virtual QVector<int>* eeprom() { return &m_eeprom; }
        virtual void setEeprom( QVector<int>* eep );
        uint32_t romSize()  { return m_romSize; }
        uint8_t  getRomValue( int address ) { return m_eeprom[address]; }
        void     setRomValue( int address, uint8_t value ) { m_eeprom[address] = value; }

        uint64_t cycle(){ return m_cycle; }

        void hardReset( bool r );
        void sleep( bool s );

        QString getFileName() { return m_firmware; }

        double freq() { return m_freq; }
        void setFreq( double freq );
        uint64_t psCycle() { return m_psCycle; }  // Simulation cycles per instruction cycle

        McuTimer* getTimer( QString name );
        McuPort* getPort( QString name );
        McuPin*  getPin( QString pinName );
        //QHash<QString, McuPort*> getPorts() { return m_portList; }

        IoPort* getIoPort( QString name );
        IoPin*  getIoPin( QString pinName );

        RamTable* getCpuTable() { return m_cpuTable; }

        McuWdt* watchDog() { return m_wdt; }
        McuVref* vrefModule();
        //McuSleep* sleepModule();

        void wdr();

        void enableInterrupts( uint8_t en );

        bool setCfgWord( uint16_t addr, uint16_t data );
        uint16_t getCfgWord( uint16_t addr=0 );

        ExtMemModule* extMem; /// REMOVE
        CpuBase* cpu;
        int cyclesDone;

        void setMain() { m_pSelf = this; }
        Component* component() { return m_component; }

        Interrupts m_interrupts;

    protected:
 static eMcu* m_pSelf;

        Component* m_component;

        void reset();

        QString m_firmware;     // firmware file loaded
        QString m_device;

        mcuState_t m_state;

        uint64_t m_cycle;
        std::vector<uint16_t> m_progMem;  // Program memory
        uint32_t m_flashSize;
        uint8_t  m_wordSize; // Size of Program memory word in bytes

        QHash<QString, int> m_regsTable;   // int max 32 bits

        uint32_t m_romSize;
        QVector<int> m_eeprom;

        QHash<uint16_t, uint16_t> m_cfgWords; // Config words

        std::vector<McuModule*> m_modules;
        std::vector<McuUsart*> m_usarts;

        QHash<QString, McuTimer*> m_timerList;// Access TIMERS by name
        QHash<QString, McuPort*>  m_portList; // Access PORTS by name
        QHash<QString, IoPort*>   m_ioPorts; // Access ioPORTS by name

        IoPin*  m_clkPin;
        McuSleep* m_sleepModule;
        McuVref* m_vrefModule;
        McuWdt* m_wdt;

        double m_freq;           // Clock Frequency in MegaHerzs
        double m_cPerInst;       // Clock ticks per Instruction Cycle
        uint64_t m_psCycle;     // Simulation cycles per Instruction Cycle

        RamTable* m_cpuTable;

        // Debugger:
        BaseDebugger* m_debugger;
        bool          m_debugging;
};

#endif
