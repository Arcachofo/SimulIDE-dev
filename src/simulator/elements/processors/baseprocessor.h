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
#include "ramtable.h"
#include "terminalwidget.h"

class BaseDebugger;

class MAINMODULE_EXPORT BaseProcessor : public QObject, public eElement
{
    Q_OBJECT
    public:
        BaseProcessor( McuComponent* parent );
        ~BaseProcessor();
        
 static BaseProcessor* self() { return m_pSelf; }

        virtual void stamp() override;
        virtual void runEvent() override;
 
        QString getFileName() { return m_symbolFile; }

        virtual void setDevice( QString device ){;}
        virtual void setDataFile( QString datafile );

        virtual bool loadFirmware( QString file )=0;
        virtual bool getLoadStatus() { return m_loadStatus; }
        virtual void terminate();

        virtual void setFreq( double freq );
        virtual void stepCpu()=0;
        virtual void reset()=0;

        virtual int pc()=0;
        virtual uint64_t cycle()=0;
        virtual int status();

        void stepOne( int line );
        
        virtual void hardReset( bool reset );
        virtual int  getRamValue( QString name );
        virtual int  getRegAddress( QString name );
        virtual void addWatchVar( QString name, int address, QString type );
        virtual void updateRamValue( QString name );

        virtual int  getRamValue( int address )=0;
        virtual void setRamValue( int address, uint8_t value )=0;
        virtual int  getFlashValue( int address )=0;
        virtual void setFlashValue( int address, uint8_t value )=0;
        virtual int  getRomValue( int address )=0;
        virtual void setRomValue( int address, uint8_t value )=0;

        virtual void uartOut( int uart, uint32_t value );
        virtual void uartIn( int uart, uint32_t value );

        virtual QStringList getRegList() { return m_regList; }
        
        virtual RamTable* getRamTable() { return &m_ramTable; }

        int ramSize() { return m_ramSize; }
        int flashSize(){ return m_flashSize; }
        int romSize(){ return m_romSize; }

        virtual QVector<int>* eeprom();
        virtual void setEeprom( QVector<int>* eep );
        
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

        int m_ramSize;
        int m_flashSize;
        int m_romSize;

        QString m_symbolFile;
        QString m_dataFile;
        QString m_device;
        QString m_statusReg;

        double m_stepPS;

        RamTable m_ramTable;

        QStringList m_regList;

        QHash<QString, int>     m_regsTable;   // int max 32 bits
        QHash<QString, QString> m_typeTable;

        QVector<int> m_eeprom;

        bool m_resetStatus;
        bool m_loadStatus;

        McuComponent* m_mcu;

        BaseDebugger* m_debugger;
        bool m_debugging;
        bool m_debugStep;
        int  m_prevLine;
};


#endif

