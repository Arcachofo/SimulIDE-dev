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

class MAINMODULE_EXPORT ScriptCpu : public ScriptModule, public McuCpu
{
    public:
        ScriptCpu( eMcu* mcu );
        ~ScriptCpu();

        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void reset();
        virtual void runStep();
        virtual void extClock( bool clkState ) override;

        virtual void setScript( QString script ) override;
        void startScript();

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

        virtual void INTERRUPT( uint vector ) override;

    protected:
        asIScriptFunction* m_reset;
        asIScriptFunction* m_voltChanged;
        asIScriptFunction* m_runEvent;
        asIScriptFunction* m_INTERRUPT;

        //asIScriptContext* m_extClockCtx;
        asIScriptFunction* m_extClock;
};
#endif
