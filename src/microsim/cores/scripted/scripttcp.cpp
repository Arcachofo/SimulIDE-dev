/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "scripttcp.h"
#include "scriptcpu.h"
#include "angelscript.h"

ScriptTcp::ScriptTcp( eMcu* mcu, QString name )
         : TcpModule( name )
         , ScriptPerif( name )
{
    m_type = "TCP";

    m_methods << "connectToHost( int link, string host, int port )"
              << "sendMsg( string msg, int link )"
              << "closeSocket( int link )";
}
ScriptTcp::~ScriptTcp(){}

void ScriptTcp::reset()
{
}

QStringList ScriptTcp::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    asIScriptEngine* engine = cpu->engine();

    string tcp = "TCP "+m_perifName.toStdString();
    engine->RegisterObjectType("TCP", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterGlobalProperty( tcp.c_str(), this );

    engine->RegisterObjectMethod("TCP", "void connectToHost( int link, string host, int port )"
                                   , asMETHODPR( ScriptTcp, connectToHost, (int,string,int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("TCP", "void sendMsgToHost( string msg, int link )"
                                   , asMETHODPR( ScriptTcp, sendMsgToHost, (string,int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("TCP", "void closeSocket( int link )"
                                   , asMETHODPR( ScriptTcp, closeSocket, (int), void)
                                   , asCALL_THISCALL );

    return m_methods;
}

void ScriptTcp::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();
    m_received  = aEngine->GetModule(0)->GetFunctionByDecl("void received( string msg, int link )");
    m_connected = aEngine->GetModule(0)->GetFunctionByDecl("void tcpConnected( int link )");
    m_closed    = aEngine->GetModule(0)->GetFunctionByDecl("void tcpDisconnected( int link )");
}

void ScriptTcp::connectToHost( int link, const string host, int port )
{
    connectTo( link, QString::fromStdString( host ), port );
}

void ScriptTcp::sendMsgToHost( const string msg, int link )
{
    sendMsg( QString::fromStdString( msg ), link );
}

// Script calls -----------------------------

void ScriptTcp::received( QString msg, int link )
{
    TcpModule::received( msg, link );
    if( !m_received ) return;

    m_scriptCpu->prepare( m_received );

    std::string str = msg.toStdString();
    m_scriptCpu->context()->SetArgObject( 0, &str );
    m_scriptCpu->context()->SetArgDWord( 1, link );
    m_scriptCpu->execute();
}

void ScriptTcp::tcpConnected( int link )
{
    TcpModule::tcpConnected( link );
    if( !m_connected ) return;

    m_scriptCpu->prepare( m_connected );
    m_scriptCpu->context()->SetArgDWord( 0, link );
    m_scriptCpu->execute();
}

void ScriptTcp::tcpDisconnected( int link )
{
    TcpModule::tcpDisconnected( link );
    if( !m_closed ) return;

    m_scriptCpu->prepare( m_closed );
    m_scriptCpu->context()->SetArgDWord( 0, link );
    m_scriptCpu->execute();
}
