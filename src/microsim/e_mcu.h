/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EMCU_H
#define EMCU_H

#include "e_iou.h"
#include "mcutimer.h"
#include "mcuuart.h"
#include "mcuinterrupts.h"
#include "mcudataspace.h"
#include "mcusleep.h"

//class CpuBase;
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
    mcuError,
    mcuRunning,
    mcuSleeping
};

class Mcu;
class McuIntOsc;
class McuPort;
class McuVref;
class Component;
class ConfigWord;
class McuComp;
class CpuBase;

class eMcu : public DataSpace, public eIou
{
        friend class McuCreator;
        friend class McuCpu;
        friend class Mcu;

    public:
        eMcu( Mcu* comp, QString id );
        ~eMcu();

 static eMcu* self() { return m_pSelf; }

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        inline mcuState_t state() { return m_state; }
        inline int sleepMode() { return m_sleepModule->mode(); }

        void stepCpu();

        void setDebugger( BaseDebugger* deb );
        void setDebugging( bool d );

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
        void start();

        QString getFileName() { return m_firmware; }

        double vdd() { return m_vdd; }

        double freq() { return m_freq; }
        void setFreq( double freq );
        uint64_t psInst() { return m_psInst; }  // picoseconds per instruction cycle
        void setInstCycle( double p ){ m_cPerInst = m_cPerTick = p; }

        McuTimer* getTimer( QString name );
        McuPort* getMcuPort( QString name );
        McuPin*  getMcuPin( QString pinName );

        IoPin*  getIoPin( QString pinName );

        McuWdt* watchDog() { return m_wdt; }
        McuVref* vrefModule();

        bool setCfgWord( uint16_t addr, uint16_t data );
        McuIntOsc* intOsc() { return m_intOsc; }
        McuComp* comparator() { return m_comparator; }

        void wdr();

        Interrupts* interrupts() { return &m_interrupts; }
        void enableInterrupts( uint8_t en );

        int cyclesDone;

        void setMain() { m_pSelf = this; }

    protected:
 static eMcu* m_pSelf;

        void reset();

        QString m_firmware;     // firmware file loaded

        mcuState_t m_state;

        uint64_t m_cycle;
        std::vector<uint16_t> m_progMem;  // Program memory
        uint32_t m_flashSize;
        uint8_t  m_wordSize; // Size of Program memory word in bytes

        QHash<QString, int> m_regsTable;   // int max 32 bits

        uint32_t m_romSize;
        QVector<int> m_eeprom;
        bool m_saveEepr;

        std::vector<McuModule*> m_modules;
        std::vector<McuUsart*> m_usarts;

        QHash<QString, McuTimer*> m_timerList;// Access TIMERS by name
        QHash<QString, McuPort*>  m_mcuPorts; // Access PORTS by name
        ConfigWord* m_cfgWord;
        McuSleep*   m_sleepModule;
        McuVref*    m_vrefModule;
        McuWdt*     m_wdt;
        McuIntOsc*  m_intOsc;
        McuComp*    m_comparator;

        Interrupts m_interrupts;

        double m_vdd;

        double m_freq;         // Clock Frequency in MegaHerzs
        double m_cPerInst;     // Clock ticks per Instruction Cycle
        double m_cPerTick;     // Clock ticks  per cpu Cycle
        uint64_t m_psInst;     // picoseconds per Instruction Cycle
        uint64_t m_psTick;     // picoseconds per Instruction Cycle

        bool m_clkState;

        // Debugger:
        BaseDebugger* m_debugger;
        bool          m_debugging;
};

#endif
