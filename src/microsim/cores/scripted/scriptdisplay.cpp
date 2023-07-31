/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "scriptdisplay.h"
#include "scriptcpu.h"
#include "simulator.h"

ScriptDisplay::ScriptDisplay( int w, int h, QString name, QWidget* parent )
             : Display( w, h, name, parent )
             , ScriptPerif()
{
    m_clear = NULL;
}
ScriptDisplay::~ScriptDisplay(){}

void ScriptDisplay::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    asIScriptEngine* engine = cpu->engine();

    asITypeInfo* info = engine->GetTypeInfoByName("Display"); // Check if "Display is already registered
    if( !info ) registerScriptMetods( engine );

    string display = "Display "+m_name.toStdString(); // Type name
    engine->RegisterGlobalProperty( display.c_str(), this );
}

void ScriptDisplay::registerScriptMetods( asIScriptEngine* engine ) // Static: register Object type and methods only once
{
    engine->RegisterObjectType("Display", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("Display", "void setWidth(int w)"
                                   , asMETHODPR( ScriptDisplay, setWidth, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setHeight(int h)"
                                   , asMETHODPR( ScriptDisplay, setHeight, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setBackground(int b)"
                                   , asMETHODPR( ScriptDisplay, setBackground, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setPixel(int x, int y, int color)"
                                   , asMETHODPR( ScriptDisplay, setPixel, (int,int,int), void)
                                   , asCALL_THISCALL );
}

void ScriptDisplay::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();

    m_clear = aEngine->GetModule(0)->GetFunctionByDecl("void clear()");
    //m_frameSent    = aEngine->GetModule(0)->GetFunctionByDecl("void frameSent( uint data )");
}

void ScriptDisplay::initialize()
{
    if( m_clear ) m_scriptCpu->callFunction( m_clear );
}
