/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scripttwi.h"
#include "scriptcpu.h"
#include "angelscript.h"

ScriptTwi::ScriptTwi( eMcu* mcu, QString name )
         : McuTwi( mcu, name )
         , ScriptPerif( name )
{
    m_byteReceived = NULL;
}
ScriptTwi::~ScriptTwi(){}

void ScriptTwi::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    asIScriptEngine* engine = cpu->engine();

    string twi = "TWI "+m_perifName.toStdString();
    engine->RegisterObjectType("TWI", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterGlobalProperty( twi.c_str(), this );

    engine->RegisterObjectMethod("TWI", "void setMode(int t)"
                                   , asMETHODPR( ScriptTwi, setMode, (twiMode_t), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("TWI", "void sendByte(uint8 b)"
                                   , asMETHODPR( ScriptTwi, sendByte, (uint8_t), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("TWI", "void setAddress(uint8 a)"
                                   , asMETHODPR( ScriptTwi, setAddress, (uint8_t), void)
                                   , asCALL_THISCALL );
}

void ScriptTwi::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();
    m_byteReceived = aEngine->GetModule(0)->GetFunctionByDecl("void byteReceived( uint d )");
}

void ScriptTwi::reset()
{
    if( m_sda ) m_sda->setPinMode( openCo );
    if( m_scl ) m_scl->setPinMode( openCo );
}

void ScriptTwi::setAddress( uint8_t a )
{
    m_cCode = m_address = a;
}

void ScriptTwi::readByte()
{
    if( !m_byteReceived ) return;

    m_scriptCpu->prepare( m_byteReceived );
    m_scriptCpu->context()->SetArgDWord( 0, m_rxReg );
    m_scriptCpu->execute();

    TwiModule::readByte();
}

void ScriptTwi::sendByte( uint8_t data )
{
    if( m_mode == TWI_SLAVE ) m_txReg = data;
    if( m_mode != TWI_MASTER ) return;

    bool write = false;
    bool isAddr = (getStaus() == TWI_START
                || getStaus() == TWI_REP_START); // We just sent Start, so this must be slave address

    if( isAddr ) write = (data & 1) == 0;        // Sending address for Read or Write?

    masterWrite( data, isAddr, write );         /// Write data or address to Slave
}
