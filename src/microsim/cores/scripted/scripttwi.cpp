/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

//#include <QDebug>

#include "scripttwi.h"
#include "scriptcpu.h"
#include "angelscript.h"

ScriptTwi::ScriptTwi( eMcu* mcu, QString name )
         : McuTwi( mcu, name )
         , ScriptPerif( name )
{
    m_byteReceived = nullptr;
    m_writeByte    = nullptr;
    m_setTwiState  = nullptr;

    m_type = "TWI";

    m_methods << "setMode( int mode )"
              << "sendByte( uint8 byte )"
              << "setAddress( uint8 address )"
              << "masterStart()"
              << "masterWrite( uint8 data, bool isAddr, bool write )"
              << "masterRead( bool ack )"
              << "masterStop()";
}
ScriptTwi::~ScriptTwi(){}

QStringList ScriptTwi::registerScript( ScriptCpu* cpu )
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

    engine->RegisterObjectMethod("TWI", "void masterStart()"
                                 , asMETHODPR( ScriptTwi, masterStart, (), void)
                                 , asCALL_THISCALL );

    engine->RegisterObjectMethod("TWI", "void masterWrite( uint8 d, bool isAddr, bool write )"
                                 , asMETHODPR( ScriptTwi, masterWrite, (uint8_t,bool,bool), void)
                                 , asCALL_THISCALL );

    engine->RegisterObjectMethod("TWI", "void masterRead( bool ack )"
                                 , asMETHODPR( ScriptTwi, masterRead, (bool), void)
                                 , asCALL_THISCALL );

    engine->RegisterObjectMethod("TWI", "void masterStop()"
                                 , asMETHODPR( ScriptTwi, masterStop, (), void)
                                 , asCALL_THISCALL );

    return m_methods;
}

void ScriptTwi::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();

    QString funcName = "void "+m_perifName+"_byteReceived( uint d )";
    m_byteReceived = aEngine->GetModule(0)->GetFunctionByDecl( funcName.toLocal8Bit().constData() );
    if( !m_byteReceived )
        m_byteReceived = aEngine->GetModule(0)->GetFunctionByDecl("void byteReceived( uint d )");

    funcName =  "void "+m_perifName+"_slaveWrite()";
    m_writeByte = aEngine->GetModule(0)->GetFunctionByDecl( funcName.toLocal8Bit().constData() );
    if( !m_writeByte )
        m_writeByte = aEngine->GetModule(0)->GetFunctionByDecl("uint slaveWrite()");

    funcName =  "void "+m_perifName+"_setTwiState( int s )";
    m_setTwiState = aEngine->GetModule(0)->GetFunctionByDecl( funcName.toLocal8Bit().constData() );
    if( !m_setTwiState )
        m_setTwiState = aEngine->GetModule(0)->GetFunctionByDecl("void setTwiState( int s )");
}

void ScriptTwi::reset()
{
}

void ScriptTwi::setAddress( uint8_t a )
{
    m_cCode = m_address = a;
}

void ScriptTwi::readByte()
{
    TwiModule::readByte();

    if( !m_byteReceived ) return;

    m_scriptCpu->prepare( m_byteReceived );
    m_scriptCpu->context()->SetArgDWord( 0, m_rxReg );
    m_scriptCpu->execute();
}

void ScriptTwi::writeByte() // Master is reading, we send byte m_txReg
{
    TwiModule::writeByte();

    if( m_mode != TWI_SLAVE ) return;
    if( !m_writeByte ) { m_txReg = 0; return; }

    m_scriptCpu->prepare( m_writeByte );
    m_scriptCpu->execute();
    m_txReg = m_scriptCpu->context()->GetReturnDWord();
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

void ScriptTwi::setTwiState( twiState_t state )
{
    TwiModule::setTwiState( state );

    if( !m_setTwiState ) return;

    m_scriptCpu->prepare( m_setTwiState );
    m_scriptCpu->context()->SetArgDWord( 0, (int)state );
    m_scriptCpu->execute();
}
