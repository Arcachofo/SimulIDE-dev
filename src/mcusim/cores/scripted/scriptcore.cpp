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

#include "scriptcore.h"


ScriptCore::ScriptCore( eMcu* mcu )
          : ScriptModule( mcu, "" )
          , McuCore( mcu )
{
}
ScriptCore::~ScriptCore() {}

void ScriptCore::reset()
{
    callFunction( &m_reset );
}

void ScriptCore::runDecoder()
{
    ;
}

void ScriptCore::runClock( bool clkState )
{
    callFunction( &m_runClock, {QScriptValue( (int)clkState )} );
}

/*void setValue( QString name, QString val )
{

}*/

void ScriptCore::setScript( QString script )
{
    ScriptModule::setScript( script );
    m_runClock = evalFunc("runClock");
}

#include "moc_scriptcore.cpp"
