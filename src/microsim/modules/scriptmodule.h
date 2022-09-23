/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTMODULE_H
#define SCRIPTMODULE_H

#include "angelscript.h"
#include "as_jit.h"

//#include "mcumodule.h"
#include "e-element.h"


class MAINMODULE_EXPORT ScriptModule : public eElement
{
    public:
        ScriptModule( QString name );
        ~ScriptModule();

        virtual void setScript( QString script );

    protected:
        int compileScript();
        void callFunction( asIScriptFunction* func, asIScriptContext* ctx=NULL );

        QString m_script;

        asCJITCompiler* m_jit;
        asIScriptEngine* m_aEngine;
        asIScriptContext* m_context;
};
#endif
