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

#ifndef SCRIPTMODULE_H
#define SCRIPTMODULE_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptProgram>

//#include "mcumodule.h"
#include "e-element.h"

class eMcu;

class MAINMODULE_EXPORT ScriptModule : public eElement, public QObject
{
    public:
        ScriptModule( eMcu *mcu, QString name );
        ~ScriptModule();

        void evalProg( QString prog );
        QScriptValue evalFunc( QString func );

        virtual void setScript( QString script );

    public slots:
        virtual void setValue( QString name, QString val ){;}

    protected:
        void getExceptions();
        QScriptValue callFunction( QScriptValue* func, QScriptValueList args=QScriptValueList() );

        QString m_script;

        QScriptEngine  m_engine;
        QScriptProgram m_program;
        QScriptValue   m_thisObject;

        QScriptValue m_reset;
        QScriptValue m_setValue;

        QScriptValue m_configureA;
        QScriptValue m_configureB;
        QScriptValue m_configureC;
};
#endif
