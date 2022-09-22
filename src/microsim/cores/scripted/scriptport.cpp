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

#include "scriptport.h"
#include "e_mcu.h"
#include "mcupin.h"

ScriptPort::ScriptPort( eMcu* mcu, QString name )
          : ScriptModule( mcu->getId()+"-"+name )
          , McuPort( mcu, name )
{
}
ScriptPort::~ScriptPort(){}

void ScriptPort::configureA( uint8_t newVal )
{
    //callFunction( &m_configureA, {QScriptValue( (int)newVal )} );
}

void ScriptPort::configureB( uint8_t newVal )
{
    //callFunction( &m_configureB, {QScriptValue( (int)newVal )} );
}

void ScriptPort::configureC( uint8_t newVal )
{
    //m_configureC.call( m_thisObject, newVal );
}

void ScriptPort::reset()
{
    //callFunction( &m_reset );
}

void ScriptPort::setExtIntTrig( int pinNumber, int trig )
{
    m_pins[pinNumber]->setExtIntTrig( trig );
}

void ScriptPort::setScript( QString script )
{
    ScriptModule::setScript( script );

    /*m_configureA = evalFunc("configureA");
    m_configureB = evalFunc("configureB");
    m_configureC = evalFunc("configureC");*/
}
