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

#include "scriptcore.h"
#include "simulator.h"
#include "ioport.h"
//#include "intmem.h"

ScriptCore::ScriptCore( eMcu* mcu )
          : ScriptModule( mcu, "" )
          , McuCpu( mcu )
{
    //m_core = core;
}
ScriptCore::~ScriptCore() {}


void ScriptCore::setScript( QString script )
{
    ScriptModule::setScript( script );
    m_extClock = evalFunc("extClock");
    m_voltChanged = evalFunc("voltChanged");
    m_runEvent = evalFunc("runEvent");
    m_INTERRUPT = evalFunc("INTERRUPT");

    callFunction( &m_setup );
}


void ScriptCore::reset()
{
    for( IoPort* port : m_ports ) port->reset();
    callFunction( &m_reset );
}

void ScriptCore::voltChanged()
{
    callFunction( &m_voltChanged );
}

void ScriptCore::runEvent()
{
    callFunction( &m_runEvent );
}

void ScriptCore::runStep()
{
    ;
}

void ScriptCore::extClock( bool clkState )
{
    callFunction( &m_extClock, {QScriptValue( (int)clkState )} );
}

/*void setValue( QString name, QString val )
{

}*/

/*void ScriptCore::getIntMem( QString name, QString scrName )
{
    IntMemModule* intMem = NULL;//m_mcu->getModule( name );
    if( !intMem ) return;

    m_intMem = m_engine.newQObject( intMem, QScriptEngine::QtOwnership
                                          , QScriptEngine::ExcludeSuperClassMethods
                                          | QScriptEngine::ExcludeSuperClassProperties );

    m_engine.globalObject().setProperty( "intMem", m_intMem );
}*/

void ScriptCore::addEvent( uint time )
{
    Simulator::self()->addEvent( time, this );
}

void ScriptCore::cancelEvents()
{
    Simulator::self()->cancelEvents( this );
}

int  ScriptCore::readPGM( uint addr )
{
    if( addr < m_progSize ) return m_progMem[addr];
    return -1;
}
void ScriptCore::writePGM( uint addr, int value )
{
    if( addr < m_progSize ) m_progMem[addr] = value;
}
int  ScriptCore::readRAM( uint addr )
{
    if( addr <= m_dataMemEnd ) return m_dataMem[addr];
    return -1;
}
void ScriptCore::writeRAM( uint addr, int value )
{
    if( addr <= m_dataMemEnd ) m_dataMem[addr] = value;
}
int  ScriptCore::readROM( uint addr )
{
    if( addr < m_mcu->romSize() ) return m_mcu->getRomValue( addr );
    return -1;
}
void ScriptCore::writeROM( uint addr, int value )
{
    if( addr < m_mcu->romSize() ) m_mcu->setRomValue( addr, value );
}

int ScriptCore::getPort( QString name )
{
    IoPort* port = m_mcu->getIoPort( name );
    if( !port ){
        qDebug() << "Error: ScriptCore::getPort Port"<< name << "Doesn't exist";
        return -1;
    }
    //port->controlPort( true, true );
    m_ports.push_back( port );

    return (m_ports.size()-1);
}

void ScriptCore::setPortMode( uint n, uint m )
{
    if( portExist( n ) ) m_ports[n]->setPinMode( (pinMode_t)m );
    else portDontExist( n );
}

void ScriptCore::setPortState( uint n, uint d )
{
    if( portExist( n ) ) m_ports[n]->setOutState( d );
    else portDontExist( n );
}

int ScriptCore::getPortState( uint n )
{
    if( portExist( n ) ) return m_ports[n]->getInpState();
    else portDontExist( n );
    return 0;
}

void ScriptCore::portVoltChanged( uint n, bool ch )
{
    if( portExist( n ) ) m_ports[n]->portVoltChanged( this, ch );
    else portDontExist( n );
}

bool ScriptCore::portExist( uint n )
{
    return ( n < m_ports.size() );
}

void ScriptCore::portDontExist( uint n )
{
    qDebug() << "Error: ScriptCore::portDontExist Port"<< n << "Doesn't exist";
}

int ScriptCore::getPin( QString name )
{
    IoPin* pin = m_mcu->getIoPin( name );
    if( !pin ){
        qDebug() << "Error: ScriptCore::getPin Pin"<< name << "Doesn't exist";
        return -1;
    }
    pin->controlPin( true, true );
    m_pins.push_back( pin );

    return (m_pins.size()-1);
}

void ScriptCore::setPinMode( uint n, uint m )
{
    if( pinExist( n ) ) m_pins[n]->setPinMode( (pinMode_t)m );
    else pinDontExist( n );
}

void ScriptCore::setPinState( uint n, uint d )
{
    if( pinExist( n ) ) m_pins[n]->setOutState( d );
    else pinDontExist( n );
}

int ScriptCore::getPinState( uint n )
{
    if( pinExist( n ) ) return m_pins[n]->getInpState();
    else pinDontExist( n );
    return 0;
}

void ScriptCore::setPinVoltage( uint n, double volt )
{
    if( pinExist( n ) )
    {
        m_pins[n]->setOutHighV( volt );
        m_pins[n]->setOutStatFast( true );
    }
    else pinDontExist( n );
}

double ScriptCore::getPinVoltage( uint n )
{
    if( pinExist( n ) ) return m_pins[n]->getVolt();
    else pinDontExist( n );
    return 0;
}

void ScriptCore::pinVoltChanged( uint n, bool ch )
{
    if( pinExist( n ) ) m_pins[n]->changeCallBack( this, ch );
    else pinDontExist( n );
}

bool ScriptCore::pinExist( uint n )
{
    return ( n < m_pins.size() );
}

void ScriptCore::pinDontExist( uint n )
{
    qDebug() << "Error: ScriptCore::pinDontExist pin"<< n << "Doesn't exist";
}

void ScriptCore::INTERRUPT( uint32_t vector )
{
    callFunction( &m_INTERRUPT, {QScriptValue( vector )} );
}

#include "moc_scriptcore.cpp"
