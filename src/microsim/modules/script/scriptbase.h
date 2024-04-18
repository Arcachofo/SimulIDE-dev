/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTBASE_H
#define SCRIPTBASE_H

#include "angelscript.h"
#include "as_jit.h"

//#include "mcumodule.h"
#include "e-element.h"

class asDebugger;

class ScriptBase : public eElement
{
    public:
        ScriptBase( QString name );
        ~ScriptBase();

        void MessageCallback( const asSMessageInfo* msg );

        virtual int compileScript();

        virtual void setScriptFile( QString scriptFile, bool compile=true );
        virtual void setScript( QString script );

        asIScriptEngine*  engine()  { return m_aEngine; }
        asIScriptContext* context() { return m_context; }

        void callFunction( asIScriptFunction* func );
        int callFunction0( asIScriptFunction* func, asIScriptContext* context );
        inline void prepare( asIScriptFunction* func ) { m_context->Prepare( func ); }
        void execute();

        void setDebugger( asDebugger* d ) { m_debugger = d; }

    protected:
        void printError( asIScriptContext* context );

        int m_status;

        QString m_script;

        asCJITCompiler* m_jit;
        asIScriptEngine* m_aEngine;
        asIScriptContext* m_context;

        asDebugger* m_debugger;
};
#endif
