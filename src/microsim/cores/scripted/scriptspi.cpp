/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scriptspi.h"
#include "scriptcpu.h"
#include "angelscript.h"
#include "usarttx.h"
#include "usartrx.h"
#include "e_mcu.h"

#define SCON *m_scon

ScriptSpi::ScriptSpi( eMcu* mcu, QString name )
         : McuSpi( mcu, name )
         , ScriptPerif()
{
    m_spiName = name;

    m_byteReceived = NULL;
}
ScriptSpi::~ScriptSpi(){}

void ScriptSpi::reset()
{
}

void ScriptSpi::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    string spi = "SPI "+m_spiName.toStdString();
    asIScriptEngine* engine = cpu->engine();

    engine->RegisterObjectType("SPI", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterGlobalProperty( spi.c_str(), this );

    engine->RegisterObjectMethod("SPI", "void setMode(int t)"
                                   , asMETHODPR( ScriptSpi, setMode, (spiMode_t), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("SPI", "void sendByte(uint8 b)"
                                   , asMETHODPR( ScriptSpi, sendByte, (uint8_t), void)
                                   , asCALL_THISCALL );
}

void ScriptSpi::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();
    m_byteReceived = aEngine->GetModule(0)->GetFunctionByDecl("void byteReceived( uint d )");
}

void ScriptSpi::byteReceived( uint8_t data )
{
    if( !m_byteReceived ) return;

    m_scriptCpu->prepare( m_byteReceived );
    m_scriptCpu->context()->SetArgDWord( 0, data );
    m_scriptCpu->execute();
}

void ScriptSpi::sendByte( uint8_t data )
{
    m_srReg = data;
    if( m_mode == SPI_MASTER ) StartTransaction();
}

void ScriptSpi::endTransaction()
{
    SpiModule::endTransaction();
    *m_dataReg = m_srReg;
    byteReceived( *m_dataReg );
}

