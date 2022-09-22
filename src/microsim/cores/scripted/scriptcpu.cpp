/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include "scriptcpu.h"
#include "angelscript.h"
#include "simulator.h"
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

    m_aEngine->RegisterObjectType("eElement" ,0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterObjectType("ScriptCpu",0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterObjectType("IoPort"   ,0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterObjectType("IoPin"    ,0, asOBJ_REF | asOBJ_NOCOUNT );
}
ScriptCpu::~ScriptCpu() {}

void ScriptCpu::setScript( QString script )
{
    m_script = script;
    if( !m_aEngine ) return;

    m_aEngine->RegisterGlobalProperty("ScriptCpu component", this );
    m_aEngine->RegisterGlobalProperty("eElement element", this );

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

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPort@ getPort(const string pin)"
                                       , asMETHODPR( ScriptCpu, getPort, (const string), IoPort*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPort", "void setPinMode(uint m)"
                                       , asMETHODPR( IoPort, setPinMode, (uint), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPort", "uint getInpState()"
                                       , asMETHODPR( IoPort, getInpState, (), uint)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPort", "void setOutState(uint s)"
                                       , asMETHODPR( IoPort, setOutState, (uint), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPort", "void changeCallBack(eElement@ s, bool s)"
                                       , asMETHODPR( IoPort, changeCallBack, (eElement*, bool), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPin@ getPin(const string pin)"
                                       , asMETHODPR( ScriptCpu, getPin, (const string), IoPin*)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "void setPinMode(uint m)"
                                       , asMETHODPR( IoPin, setPinMode, (uint), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "bool getInpState()"
                                       , asMETHODPR( IoPin, getInpState, (), bool)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "void setOutState(bool s)"
                                       , asMETHODPR( IoPin, setOutState, (bool), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "double getVolt()"
                                       , asMETHODPR( IoPin, getVolt, (), double)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "void setVoltage(double v)"
                                       , asMETHODPR( IoPin, setVoltage, (double), void)
                                       , asCALL_THISCALL );

    r = m_aEngine->RegisterObjectMethod("IoPin", "void changeCallBack(eElement@ p, bool s)"
                                       , asMETHODPR( IoPin, changeCallBack, (eElement*, bool), void)
                                       , asCALL_THISCALL );

    r = compileScript();
    if( r < 0 ) return;

    asIScriptFunction* func = m_aEngine->GetModule(0)->GetFunctionByDecl("void setup()");
    if( func ) callFunction( func );

    m_reset       = m_aEngine->GetModule(0)->GetFunctionByDecl("void reset()");
    m_voltChanged = m_aEngine->GetModule(0)->GetFunctionByDecl("void voltChanged()");
    m_runEvent    = m_aEngine->GetModule(0)->GetFunctionByDecl("void runEvent()");
    m_extClock    = m_aEngine->GetModule(0)->GetFunctionByDecl("void extClock()");
    m_extClockCtx = m_aEngine->CreateContext();
}

void ScriptCpu::reset()
{
    //for( IoPort* port : m_ports ) port->reset();
    callFunction( m_reset );
}
void ScriptCpu::voltChanged() { callFunction( m_voltChanged ); }
void ScriptCpu::runEvent()    { callFunction( m_runEvent ); }
void ScriptCpu::runStep()     { ; }
void ScriptCpu::extClock( bool clkState ) { if( clkState ) callFunction( m_extClock, m_extClockCtx ); }

void ScriptCpu::addEvent( uint time ) { Simulator::self()->addEvent( time, this ); }
void ScriptCpu::cancelEvents()        { Simulator::self()->cancelEvents( this ); }

int  ScriptCpu::readPGM( uint addr )         { if( addr < m_progSize       ) return m_progMem[addr]; return -1; }
void ScriptCpu::writePGM( uint addr, int v ) { if( addr < m_progSize       ) m_progMem[addr] = v; }
int  ScriptCpu::readRAM( uint addr )         { if( addr <= m_dataMemEnd    ) return m_dataMem[addr]; return -1; }
void ScriptCpu::writeRAM( uint addr, int v ) { if( addr <= m_dataMemEnd    ) m_dataMem[addr] = v; }
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

void   ScriptCpu::INTERRUPT( uint32_t vector )
{ //callFunction( &m_INTERRUPT, {QScriptValue( vector )} );
}

