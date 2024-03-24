/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "scriptmodule.h"
#include "mcu.h"

ScriptModule::ScriptModule( eMcu* mcu, QString name )
            : ScriptBase( mcu->getId()+"-"+"ScriptModule" )
            , McuModule( mcu, name )
{
    m_configureA   = NULL;
    m_configureB   = NULL;
    m_configureC   = NULL;
    m_callBackDoub = NULL;
    m_callBack     = NULL;
}
ScriptModule::~ScriptModule()
{
}

void ScriptModule::setScriptFile( QString scriptFile, bool compile )
{
    ScriptBase::setScriptFile( scriptFile, compile );
    if( compile ) compileScript();
}

void ScriptModule::setScript( QString script )
{
    ScriptBase::setScript( script  );
}

int ScriptModule::compileScript()
{
    if( !m_aEngine ) return -1;

    int r = ScriptBase::compileScript();
    if( r < 0 ) return r;

    m_configureA   = m_aEngine->GetModule(0)->GetFunctionByDecl("void configureA( uint v )");
    m_configureB   = m_aEngine->GetModule(0)->GetFunctionByDecl("void configureB( uint v )");
    m_configureC   = m_aEngine->GetModule(0)->GetFunctionByDecl("void configureC( uint v )");
    m_callBackDoub = m_aEngine->GetModule(0)->GetFunctionByDecl("void callBackDoub( double v )");
    m_callBack     = m_aEngine->GetModule(0)->GetFunctionByDecl("void callBack()");

    //m_vChangedCtx = m_voltChanged ? m_aEngine->CreateContext() : NULL;

    asIScriptFunction* func = m_aEngine->GetModule(0)->GetFunctionByDecl("void setup()");
    if( func ) callFunction( func );

    return 0;
}

void ScriptModule::configureA( uint8_t v )
{
    if( !m_configureA ) return;

    prepare( m_configureA );
    m_context->SetArgDWord( 0, v );
    execute();
}

void ScriptModule::configureB( uint8_t v )
{
    if( !m_configureB ) return;

    prepare( m_configureB );
    m_context->SetArgDWord( 0, v );
    execute();
}

void ScriptModule::configureC( uint8_t v )
{
    if( !m_configureC ) return;

    prepare( m_configureC );
    m_context->SetArgDWord( 0, v );
    execute();
}

void ScriptModule::callBackDoub( double v )
{
    if( !m_callBackDoub ) return;

    prepare( m_callBackDoub );
    m_context->SetArgDouble( 0, v );
    execute();
}

void ScriptModule::callBack()
{
    if( !m_callBack ) return;

    prepare( m_callBack );
    execute();
}
