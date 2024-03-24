/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SCRIPTMODULE_H
#define SCRIPTMODULE_H

#include "mcumodule.h"
#include "scriptbase.h"
//#include "e-element.h"

class ScriptModule : public ScriptBase, public McuModule //eElement
{
    public:
        ScriptModule( eMcu* mcu, QString name );
        ~ScriptModule();

        virtual int compileScript() override;

        virtual void setScriptFile( QString scriptFile, bool compile=true ) override;
        virtual void setScript( QString script ) override;

        virtual void configureA( uint8_t ) override;
        virtual void configureB( uint8_t ) override;
        virtual void configureC( uint8_t ) override;
        virtual void callBackDoub( double )override;
        virtual void callBack()            override;

    protected:

        asIScriptFunction* m_configureA;
        asIScriptFunction* m_configureB;
        asIScriptFunction* m_configureC;
        asIScriptFunction* m_callBackDoub;
        asIScriptFunction* m_callBack;
};
#endif
