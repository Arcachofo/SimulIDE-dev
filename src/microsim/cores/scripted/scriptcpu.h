/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "scriptbase.h"
#include "iopin.h"
#include "mcu8bits.h"

using namespace std;

class ScriptPerif;
class Mcu;

class ScriptCpu : public ScriptBase, public Mcu8bits
{
    friend class McuCreator;

    public:
        ScriptCpu( eMcu* mcu );
        ~ScriptCpu();

        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void reset() override;
        virtual void runStep() override;
        virtual void extClock( bool clkState ) override;
        virtual void updateStep() override;

        virtual void setScriptFile( QString scriptFile, bool compile=true ) override;
        virtual int compileScript() override;

        virtual void command( QString c ) override;
        void toConsole( string r );
        void showValue( string r );

        void addCpuReg( string name, string type );
        void addCpuVar( string name, string type );
        virtual int getIntReg( QString reg ) override;
        virtual QString getStrReg( QString ) override;

        void setLinkedVal( double v, int i=0 );                       // Called from C++
        void setLinkedStr( QString s, int i );                        // Called from C++

        void RETI();
        virtual void INTERRUPT( uint vector ) override;

        void mousePress  ( int x, int y, int button );
        void mouseRelease( int x, int y, int button );
        void mouseMoved  ( int x, int y );

        QStringList getTypes() { return m_types; }
        QMap<QString, QStringList> getMemberWords() { return m_memberWords; }

    protected:
        void setPeriferals( std::vector<ScriptPerif*> p);
        ComProperty* addProperty( QString name, QString label, QString type, QString unit );
        QString getProp( ComProperty* p );
        void setProp( ComProperty* p, QString val );

        void addEvent( uint64_t time );
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

        void setPackageSize( int width, int height );                 // Called from script: Set Package size
        void setMargins( int top, int bottom, int right, int left );  // Called from script: set Margins for display
        void setBackground( const string b );

        string getPropStr( int index, const string p );               // Called from script: Get property p from linked component at index
        void setPropStr( int index, const string p, const string v ); // Called from script: Set property p with value v in linked component at index
        void setLinkedValue( int index, double v, int i=0  );         // Called from script
        void setLinkedString( int index, const string str, int i=0 ); // Called from script

        uint m_progWordMask;

        Mcu* m_mcuComp;
        asIScriptFunction* m_initialize;
        asIScriptFunction* m_reset;
        asIScriptFunction* m_voltChanged;
        asIScriptFunction* m_updateStep;
        asIScriptFunction* m_runEvent;
        asIScriptFunction* m_runStep;
        asIScriptFunction* m_extClock;
        asIScriptFunction* m_extClockF;
        asIScriptFunction* m_INTERRUPT;

        asIScriptFunction* m_getIntReg;
        asIScriptFunction* m_getStrReg;
        asIScriptFunction* m_command;

        asIScriptFunction* m_setLinkedVal;
        asIScriptFunction* m_setLinkedStr;

        asIScriptFunction* m_mousePress;
        asIScriptFunction* m_mouseRelease;
        asIScriptFunction* m_mouseMoved;

        asIScriptContext* m_vChangedCtx;
        asIScriptContext* m_runEventCtx;
        asIScriptContext* m_runStepCtx;
        asIScriptContext* m_extClockCtx;

        std::vector<ComProperty*> m_scriptProps;
        QMap<QString, QString> m_propFunctions;
        QMap<QString, asIScriptFunction*> m_propGetters;
        QMap<QString, asIScriptFunction*> m_propSetters;

        QStringList m_types;
        QMap<QString, QStringList> m_typeWords;
        QMap<QString, QStringList> m_memberWords;

        QString m_value;

        std::vector<ScriptPerif*> m_periferals;
};
