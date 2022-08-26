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
#include "iopin.h"
#include "mcucpu.h"

//class IntMemModule;

class MAINMODULE_EXPORT ScriptCore : public ScriptModule, public McuCpu
{
    Q_OBJECT
    public:
        ScriptCore( eMcu* mcu );
        ~ScriptCore();

        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void reset();
        virtual void runStep();
        virtual void extClock( bool clkState ) override;

        virtual void setScript( QString script ) override;

    public slots:
        void addEvent( uint time );
        void cancelEvents();

        int  readPGM( uint addr );
        void writePGM( uint addr, int value );
        int  readRAM( uint addr );
        void writeRAM( uint addr, int value );
        int  readROM( uint addr );
        void writeROM( uint addr, int value );

        int  getPort( QString name );
        void setPortMode( uint n, uint m );
        void setPortState( uint n, uint d );
        int  getPortState( uint n );
        void portVoltChanged( uint n, bool ch );

        int  getPin( QString name );
        void setPinMode( uint n, uint m );
        void setPinState( uint n, uint d );
        int  getPinState( uint n );
        void setPinVoltage( uint n, double volt );
        double getPinVoltage( uint n );
        void pinVoltChanged( uint n, bool ch );

        virtual void INTERRUPT( uint32_t vector ) override;

        //virtual void setValue( QString name, int val ) override;
        //void getIntMem( QString name , QString scrName );

    protected:
        inline bool portExist( uint n );
        inline void portDontExist( uint n );
        inline bool pinExist( uint n );
        inline void pinDontExist( uint n );

        //CpuBase* m_core;
        QScriptValue m_extClock;
        QScriptValue m_voltChanged;
        QScriptValue m_runEvent;
        QScriptValue m_INTERRUPT;

        std::vector<IoPort*> m_ports;
        std::vector<IoPin*> m_pins;

        //IntMemModule* m_intMem;
        //QScriptValue  m_intMem;
};
#endif
