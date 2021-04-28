/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "e-element.h"

#include "terminalwidget.h"
#include "mcuinterface.h"

class BaseDebugger;

class MAINMODULE_EXPORT BaseProcessor : public QObject, public McuInterface
{
    Q_OBJECT
    public:
        BaseProcessor( McuComponent* parent );
        ~BaseProcessor();
        
 static BaseProcessor* self() { return m_pSelf; }

        virtual void stamp() override;
        virtual void runEvent() override;

        virtual void setDevice( QString device ){;}
        virtual void setDataFile( QString datafile );

        virtual bool loadFirmware( QString file )=0;
        virtual bool getLoadStatus() { return m_loadStatus; }
        virtual void terminate();

        virtual void setFreq( double freq );
        virtual void stepCpu()=0;
        virtual void reset()=0;

        virtual uint64_t cycle()=0;
        virtual int status() override;

        void stepOne( int line );
        
        virtual void hardReset( bool reset );
        virtual uint8_t getRamValue( QString name );
        virtual uint8_t getRamValue( int address ) override {return 0;} // Implemented in child classes
        virtual uint16_t getRegAddress( QString name ) override;
        virtual void addWatchVar( QString name, int address, QString type );

        virtual void uartOut( int uart, uint32_t value );
        virtual void uartIn( int uart, uint32_t value );

        virtual QStringList getRegList() override { return m_regList; }

        virtual void setRegisters();

        void setDebugging( bool d ) { m_debugging = d; }
        void setDebugger( BaseDebugger* deb );

        void setMain() { m_pSelf = this; }

    signals:
        void uartDataOut( int uart, int value );
        void uartDataIn(  int uart, int value );
    
    protected:
 static BaseProcessor* m_pSelf;
        
        virtual int validate( int address ) { return address; }

        QString m_firmware;
        QString m_dataFile;
        QString m_device;
        QString m_statusReg;

        double m_stepPS;

        QHash<QString, int> m_regsTable;   // int max 32 bits

        bool m_resetStatus;
        bool m_loadStatus;

        McuComponent* m_mcu;

        BaseDebugger* m_debugger;
        bool m_debugging;
        bool m_debugStep;
        int  m_prevLine;
};


#endif

