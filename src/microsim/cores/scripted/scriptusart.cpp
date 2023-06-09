/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scriptusart.h"
#include "scriptcpu.h"
#include "angelscript.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"

#define SCON *m_scon

ScriptUsart::ScriptUsart( eMcu* mcu, QString name, int number )
           : McuUsart( mcu, name, number )
           , ScriptPerif()
{
    m_uartName = name;

    m_byteReceived = NULL;
    m_frameSent = NULL;
}
ScriptUsart::~ScriptUsart(){}

void ScriptUsart::reset()
{
    m_sender->enable( true );
    m_receiver->enable( true );
}

void ScriptUsart::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    string uart = "Uart "+m_uartName.toStdString();
    asIScriptEngine* engine = cpu->engine();

    engine->RegisterObjectType("Uart",0 , asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterGlobalProperty( uart.c_str(), this );

    engine->RegisterObjectMethod("Uart", "void setBaudRate(int t)"
                                   , asMETHODPR( ScriptUsart, setBaudRate, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Uart", "void setDataBits(uint8 b)"
                                   , asMETHODPR( ScriptUsart, setDataBits, (uint8_t), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Uart", "void sendByte(uint8 b)"
                                   , asMETHODPR( ScriptUsart, sendByte, (uint8_t), void)
                                   , asCALL_THISCALL );
}

void ScriptUsart::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();
    m_byteReceived = aEngine->GetModule(0)->GetFunctionByDecl("void byteReceived( uint d )");
    m_frameSent    = aEngine->GetModule(0)->GetFunctionByDecl("void frameSent( uint data )");
}

void ScriptUsart::byteReceived( uint8_t data )
{
    if( !m_byteReceived ) return;

    data = m_receiver->getData();
    m_scriptCpu->prepare( m_byteReceived );
    m_scriptCpu->context()->SetArgDWord( 0, data );
    m_scriptCpu->execute();
}

void ScriptUsart::frameSent( uint8_t data )
{
    if( !m_frameSent ) return;

    m_scriptCpu->prepare( m_frameSent );
    m_scriptCpu->context()->SetArgDWord( 0, data );
    m_scriptCpu->execute();
}
