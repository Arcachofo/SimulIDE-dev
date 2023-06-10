/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTCPU_H
#define SCRIPTCPU_H

#include "scriptmodule.h"
#include "iopin.h"
#include "mcucpu.h"


using namespace std;

class ScriptPerif;
class Mcu;

class MAINMODULE_EXPORT ScriptCpu : public ScriptModule, public McuCpu
{
    public:
        ScriptCpu( eMcu* mcu );
        ~ScriptCpu();

        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void reset() override;
        virtual void runStep() override;
        virtual void extClock( bool clkState ) override;
        virtual void updateStep() override;

        void setPeriferals( std::vector<ScriptPerif*> p);
        virtual void setScriptFile( QString scriptFile, bool compile=true ) override;
        virtual int compileScript() override;

        void command( QString c );
        void toConsole( string r );
        void showValue( string r );

        void addCpuReg( string name, string type );
        void addCpuVar( string name, string type );
        virtual int getCpuReg( QString reg ) override;
        virtual QString getStrReg( QString ) override;

        void addEvent( uint time );
        void cancelEvents();
        uint64_t circTime();

        int  readPGM( uint addr );
        void writePGM( uint addr, int value );
        int  readRAM( uint addr );
        void writeRAM( uint addr, int value );
        int  readROM( uint addr );
        void writeROM( uint addr, int value );

        IoPort* getPort( const string portName );
        IoPin*  getPin( const string pinName );

        McuPort* getMcuPort( const string portName );
        McuPin*  getMcuPin( const string pinName );

        void setLinkable();
        void setLinkedValue( int index, int v );

        virtual void INTERRUPT( uint vector ) override;

    protected:
        Mcu* m_mcuComp;
        asIScriptFunction* m_reset;
        asIScriptFunction* m_voltChanged;
        asIScriptFunction* m_runEvent;
        asIScriptFunction* m_extClock;
        asIScriptFunction* m_INTERRUPT;

        asIScriptFunction* m_getCpuReg;
        asIScriptFunction* m_getStrReg;
        asIScriptFunction* m_command;

        //asIScriptContext* m_extClockCtx;

        QString m_value;

        Watcher* m_watcher;

        std::vector<ScriptPerif*> m_periferals;
};
#endif
