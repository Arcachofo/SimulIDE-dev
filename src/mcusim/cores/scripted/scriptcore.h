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

#ifndef SCRIPTCORE_H
#define SCRIPTCORE_H

#include "scriptmodule.h"
#include "mcucore.h"

//class IntMemModule;

class MAINMODULE_EXPORT ScriptCore : public ScriptModule//, public McuCore
{
    Q_OBJECT
    public:
        ScriptCore( eMcu* mcu );
        ~ScriptCore();

        virtual void reset();
        virtual void runDecoder();
        virtual void runClock( bool clkState );

        virtual void setScript( QString script ) override;

    public slots:
        //virtual void setValue( QString name, int val ) override;
        void getIntMem( QString name , QString scrName );

    protected:
        McuCore* m_core;
        QScriptValue m_runClock;

        //IntMemModule* m_intMem;
        QScriptValue  m_intMem;
};
#endif
