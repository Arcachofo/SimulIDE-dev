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

        virtual void command( QString c ) override;
        void toConsole( string r );
        void showValue( string r );

        void addCpuReg( string name, string type );
        void addCpuVar( string name, string type );
        virtual int getCpuReg( QString reg ) override;
        virtual QString getStrReg( QString ) override;

        void addProperty( QString group, QString name, QString type );
        QString getProp( ComProperty* p );
        void setProp( ComProperty* p, QString val );

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

        string getPropStr( int index, const string p );               // Called from script:Get property p from linked component at index
        void setLinkedValue( int index, double v, int i=0  );            // Called from script
        void setLinkedString( int index, const string str, int i=0 ); // Called from script
        void setLinkedVal( double v, int i=0 );                          // Called from C++
        void setLinkedStr( QString s, int i );                        // Called from C++

        virtual void INTERRUPT( uint vector ) override;

    protected:
        Mcu* m_mcuComp;
        asIScriptFunction* m_reset;
        asIScriptFunction* m_voltChanged;
        asIScriptFunction* m_updateStep;
        asIScriptFunction* m_runEvent;
        asIScriptFunction* m_runStep;
        asIScriptFunction* m_extClock;
        asIScriptFunction* m_extClockF;
        asIScriptFunction* m_INTERRUPT;

        asIScriptFunction* m_getCpuReg;
        asIScriptFunction* m_getStrReg;
        asIScriptFunction* m_command;

        asIScriptFunction* m_setLinkedVal;
        asIScriptFunction* m_setLinkedStr;

        asIScriptContext* m_vChangedCtx;
        asIScriptContext* m_runEventCtx;
        asIScriptContext* m_runStepCtx;
        asIScriptContext* m_extClockCtx;

        std::vector<ComProperty*> m_scriptProps;
        QHash<QString, QString> m_propFunctions;
        QHash<QString, asIScriptFunction*> m_propGetters;
        QHash<QString, asIScriptFunction*> m_propSetters;

        QString m_value;

        Watcher* m_watcher;

        std::vector<ScriptPerif*> m_periferals;
};
#endif
