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

#include "scriptmodule.h"
#include "e_mcu.h"

ScriptModule::ScriptModule( eMcu* mcu, QString name )
            : eElement( mcu->getId()+"-"+name )
            , QObject()
{
    m_thisObject = m_engine.newQObject( this, QScriptEngine::QtOwnership
                                            , QScriptEngine::ExcludeSuperClassMethods
                                            | QScriptEngine::ExcludeSuperClassProperties );
    //m_engine.globalObject().setProperty( "component", m_thisObject );

}
ScriptModule::~ScriptModule() {}

void ScriptModule::evalProg( QString prog )
{
    if( !m_engine.canEvaluate( prog ) ) qDebug() << "ScriptModule::evalProg Error: can't evaluate script";

    m_program = QScriptProgram( prog );
    m_engine.evaluate( m_program );

    getExceptions();
}

QScriptValue ScriptModule::evalFunc( QString func )
{
    QScriptValue function;
    if( !m_script.contains("function "+func+"(") ) return function;

   function = m_engine.evaluate( func );
    if( !function.isFunction() ) qDebug()<<"ScriptModule::evalFunc"<<func<<" is not a function:\n";

    //function.call( m_thisObject );
    getExceptions();

    return function;
}

void ScriptModule::getExceptions()
{
    if( m_engine.hasUncaughtException() )
    {
        QScriptValue exception = m_engine.uncaughtException();
        qDebug() << exception.toString();
    }
}

QScriptValue ScriptModule::callFunction( QScriptValue* func, QScriptValueList args )
{
    QScriptValue ret = func->call( m_thisObject, args );
    getExceptions();
    return ret;
}

void ScriptModule::setScript( QString script )
{
    m_script = script;
    evalProg( script );

    m_init     = evalFunc("init");
    m_reset    = evalFunc("reset");
    m_setValue = evalFunc("setValue");

    m_configureA = evalFunc("configureA");
    m_configureB = evalFunc("configureB");
    m_configureC = evalFunc("configureC");
}
