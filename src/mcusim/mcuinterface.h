/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef MCUINTERFACE_H
#define MCUINTERFACE_H

#include "e-element.h"
#include "ramtable.h"

class MAINMODULE_EXPORT McuInterface : public eElement
{

    public:
        McuInterface( QString id );
        ~McuInterface();

        virtual uint8_t getRamValue( int address )=0;
        virtual void    setRamValue( int address, uint8_t value )=0;
        virtual uint16_t getFlashValue( int address )=0;
        virtual void     setFlashValue( int address, uint16_t value )=0;
        virtual uint8_t getRomValue( int address )=0;
        virtual void    setRomValue( int address, uint8_t value )=0;

        virtual uint16_t getRegAddress( QString name ) =0;

        virtual int status()=0;
        virtual int pc()=0;

        virtual QStringList getRegList() { return m_regList; }

        virtual QVector<int>* eeprom();
        virtual void setEeprom( QVector<int>* eep );

        uint32_t ramSize()  { return m_ramSize; }
        uint32_t flashSize(){ return m_flashSize; }
        uint32_t romSize()  { return m_romSize; }
        uint32_t wordSize() { return m_wordSize; }

        virtual void updateRamValue( QString name );

        QString getFileName() { return m_firmware; }
        virtual RamTable* getRamTable() { return m_ramTable; }

    protected:

        QString m_firmware;     // firmware file loaded

        QStringList m_regList;  // List of Register names

        QVector<int> m_eeprom;

        QHash<QString, QString> m_typeTable;

        uint32_t m_ramSize;
        uint32_t m_flashSize;
        uint32_t m_romSize;
        uint8_t  m_wordSize; // Size of Program memory word in bytes

        RamTable* m_ramTable;
};


#endif

