/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "scriptcpu.h"
#include "angelscript.h"
#include "simulator.h"
#include "mcupin.h"
#include "mcuport.h"
#include "ioport.h"

using namespace std;

ScriptCpu::ScriptCpu( eMcu* mcu )
         : ScriptModule( mcu->getId()+"-"+"ScriptCpu" )
         , McuCpu( mcu )
{
    m_reset = NULL;
    m_voltChanged = NULL;
    m_runEvent = NULL;
    m_extClock = NULL;
    m_INTERRUPT = NULL;

    m_aEngine->RegisterObjectType("eElement" ,0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterObjectType("ScriptCpu",0, asOBJ_REF | asOBJ_NOCOUNT );
    //m_aEngine->RegisterObjectType("pod", sizeof(pod), asOBJ_VALUE | asOBJ_POD);

    m_aEngine->RegisterGlobalProperty("ScriptCpu component", this );
    m_aEngine->RegisterGlobalProperty("eElement element", this );

    IoPort::registerScript( m_aEngine );
    IoPin::registerScript( m_aEngine );
    McuPort::registerScript( m_aEngine );
    McuPin::registerScript( m_aEngine );

    int r;
    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void addEvent(uint t)"
                                       , asMETHODPR( ScriptCpu, addEvent, (uint), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void cancelEvents()"
                                       , asMETHOD( ScriptCpu, cancelEvents )
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "int readPGM(uint n)"
                                       , asMETHODPR( ScriptCpu, readPGM, (uint), int)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void writePGM(uint a, int v)"
                                       , asMETHODPR( ScriptCpu, writePGM, (uint, int), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "int readRAM(uint n)"
                                       , asMETHODPR( ScriptCpu, readRAM, (uint), int)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void writeRAM(uint a, int v)"
                                       , asMETHODPR( ScriptCpu, writeRAM, (uint, int), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "int readROM(uint n)"
                                       , asMETHODPR( ScriptCpu, readROM, (uint), int)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void writeROM(uint a, int v)"
                                       , asMETHODPR( ScriptCpu, writeROM, (uint, int), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPort@ getPort(const string port)"
                                       , asMETHODPR( ScriptCpu, getPort, (const string), IoPort*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPin@ getPin(const string pin)"
                                       , asMETHODPR( ScriptCpu, getPin, (const string), IoPin*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "McuPort@ getMcuPort(const string port)"
                                       , asMETHODPR( ScriptCpu, getMcuPort, (const string), McuPort*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "McuPin@ getMcuPin(const string pin)"
                                       , asMETHODPR( ScriptCpu, getMcuPin, (const string), McuPin*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "void INTERRUPT( uint vector )"
                                       , asMETHODPR( ScriptCpu, INTERRUPT, (uint32_t), void)
                                       , asCALL_THISCALL );
}
ScriptCpu::~ScriptCpu() {}

void ScriptCpu::setScript( QString script )
{
    if( !m_aEngine ) return;
    m_script = script;

    int r = compileScript();
    if( r < 0 ) return;
    startScript();
}

void ScriptCpu::startScript()
{
    asIScriptFunction* func = m_aEngine->GetModule(0)->GetFunctionByDecl("void setup()");
    if( func ) callFunction( func );

    m_reset       = m_aEngine->GetModule(0)->GetFunctionByDecl("void reset()");
    m_voltChanged = m_aEngine->GetModule(0)->GetFunctionByDecl("void voltChanged()");
    m_runEvent    = m_aEngine->GetModule(0)->GetFunctionByDecl("void runEvent()");
    m_INTERRUPT   = m_aEngine->GetModule(0)->GetFunctionByDecl("void INTERRUPT( uint vector )");
    m_extClock    = m_aEngine->GetModule(0)->GetFunctionByDecl("void extClock( bool clkState )");
    //m_extClockCtx = m_aEngine->CreateContext();
}

void ScriptCpu::reset()
{
    //for( IoPort* port : m_ports ) port->reset();
    callFunction( m_reset );
}
void ScriptCpu::voltChanged() { callFunction( m_voltChanged ); }
void ScriptCpu::runEvent()    { callFunction( m_runEvent ); }
void ScriptCpu::INTERRUPT( uint vector )
{
    prepare( m_INTERRUPT );
    m_context->SetArgDWord( 0, vector );
    //m_context->SetArgAddress( 0, &vector ); // Not working
    execute();
}
void ScriptCpu::runStep()     { ; }
void ScriptCpu::extClock( bool clkState )
{
    prepare( m_extClock );
    m_context->SetArgByte( 0, clkState );
    execute();
}

void ScriptCpu::addEvent( uint time ) { Simulator::self()->addEvent( time, this ); }
void ScriptCpu::cancelEvents()        { Simulator::self()->cancelEvents( this ); }

int  ScriptCpu::readPGM( uint addr )         { if( addr < m_progSize       ) return m_progMem[addr]; return -1; }
void ScriptCpu::writePGM( uint addr, int v ) { if( addr < m_progSize       ) m_progMem[addr] = v; }
int  ScriptCpu::readRAM( uint addr )         { if( addr <= m_dataMemEnd    ) return m_dataMem[addr]; return -1; }
void ScriptCpu::writeRAM( uint addr, int v ) { SET_RAM( addr, v ); }
int  ScriptCpu::readROM( uint addr )         { if( addr < m_mcu->romSize() ) return m_mcu->getRomValue( addr ); return -1; }
void ScriptCpu::writeROM( uint addr, int v ) { if( addr < m_mcu->romSize() ) m_mcu->setRomValue( addr, v ); }

IoPort* ScriptCpu::getPort( const string portName )
{
    QString name = QString::fromStdString( portName );
    IoPort* port = m_mcu->getIoPort( name );
    if( !port ) qDebug() << "Error: ScriptCpu::getPort Port"<< name << "Doesn't exist";
    return port;
}

IoPin* ScriptCpu::getPin( const string pinName )
{
    QString name = QString::fromStdString( pinName );
    IoPin* pin = m_mcu->getIoPin( name );
    if( !pin ) qDebug() << "Error: ScriptCpu::getPin Pin"<< name << "Doesn't exist";
    return pin;
}

McuPort* ScriptCpu::getMcuPort( const string portName )
{
    QString name = QString::fromStdString( portName );
    McuPort* port = m_mcu->getPort( name );
    if( !port ) qDebug() << "Error: ScriptCpu::getMcuPort Port"<< name << "Doesn't exist";
    return port;
}

McuPin* ScriptCpu::getMcuPin( const string pinName )
{
    QString name = QString::fromStdString( pinName );
    McuPin* pin = m_mcu->getPin( name );
    if( !pin ) qDebug() << "Error: ScriptCpu::getMcuPin Pin"<< name << "Doesn't exist";
    return pin;
}
