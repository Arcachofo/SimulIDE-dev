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

#include <QDebug>

#include "mcudataspace.h"
#include "datautils.h"
#include "utils.h"

DataSpace::DataSpace()
{
    m_ramSize   = 0;
    m_regStart = 0xFFFF;
    m_regEnd   = 0;
}

DataSpace::~DataSpace()
{
    for( uint16_t addr : m_regSignals.keys() )
        delete m_regSignals.value( addr );

    m_regSignals.clear();
    m_dataMem.clear();
}

void DataSpace::initialize()
{
    std::fill( m_dataMem.begin(), m_dataMem.end(), 0 );

    for( QString regName : m_regInfo.keys() )  // Set Registers Reset Values
    {
        regInfo_t regInfo = m_regInfo.value(regName);
        writeReg( regInfo.address, regInfo.resetVal, false );
        m_dataMem[regInfo.address] = regInfo.resetVal;
}   }

uint8_t DataSpace::readReg( uint16_t addr )
{
    uint8_t v = m_dataMem[addr];
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal )
    {
        m_regOverride = -1;
        regSignal->on_read.emitValue( v );
        if( m_regOverride >= 0 ) v = (uint8_t)m_regOverride; // Value overriden in callback
        else                     v = m_dataMem[addr]; // Timers update their counters in callback
    }
    return v;
}

void DataSpace::writeReg( uint16_t addr, uint8_t v, bool masked )
{
    if( masked ) // Protect Read Only bits from being written
    {
        uint8_t mask = 255;
        if( addr < m_regMask.size() ) mask = m_regMask[addr];
        if( mask != 0xFF ) v = (m_dataMem[addr] & ~mask) | (v & mask);
    }
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal )
    {
        m_regOverride = -1;
        regSignal->on_write.emitValue( v );
        if( m_regOverride >= 0 ) v = (uint8_t)m_regOverride; // Value overriden in callback
    }
    m_dataMem[addr] = v;
}

uint16_t DataSpace::getRegAddress( QString reg )// Get Reg address by name
{
    uint16_t addr =  65535;
    if( m_regInfo.contains( reg ) ) addr = m_regInfo.value( reg ).address;
    else if( m_regInfo.contains( reg.toUpper() ) ) addr = m_regInfo.value( reg ).address;

    return addr;
}

uint8_t* DataSpace::getReg( QString reg )                // Get pointer to Reg data by name
{
    if( !m_regInfo.contains( reg ) )
    {
        qDebug() << "Warning: Register not found:" << reg <<"\n";
        return NULL;
    }
    return &m_dataMem[m_regInfo.value( reg ).address];
}

uint8_t DataSpace::getRamValue( int address ) { return readReg( getMapperAddr(address) ); }

void DataSpace::setRamValue( int address, uint8_t value ) // Setting RAM from external source (McuMonitor)
{ writeReg( getMapperAddr(address), value ); }

void DataSpace::updateRamValue( QString name )
{
    name = name.toUpper();
    QString type = "u8";
    if( m_typeTable.contains( name )) type = m_typeTable[ name ];

    QByteArray ba;
    ba.resize(4);
    int address = getRegAddress( name );
    if( address < 0 ) return;

    int bits = 8;

    if( type.contains( "32" ) )    // 4 bytes
    {
        bits = 32;
        ba[0] = getRamValue( address );
        ba[1] = getRamValue( address+1 );
        ba[2] = getRamValue( address+2 );
        ba[3] = getRamValue( address+3 );
    }
    else if( type.contains( "16" ) )  // 2 bytes
    {
        bits = 16;
        ba[0] = getRamValue( address );
        ba[1] = getRamValue( address+1 );
        ba[2] = 0;
        ba[3] = 0;
    }else{                             // 1 byte
        ba[0] = getRamValue( address );
        ba[1] = 0;
        ba[2] = 0;
        ba[3] = 0;
    }
    if( type.contains( "f" ) )        // float, double
    {
        float value = 0;
        memcpy(&value, ba, 4);
        m_ramTable->setItemValue( 2, value  );
    }
    else{                             // char, int, long
        int32_t value = 0;

        if( type.contains( "u" ) )
        {
            uint32_t val = 0;
            memcpy(&val, ba, 4);
            value = val;
        }else{
            if( bits == 32 )
            {
                int32_t val = 0;
                memcpy(&val, ba, 4);
                value = val;
            }
            else if( bits == 16 )
            {
                int16_t val = 0;
                memcpy(&val, ba, 2);
                value = val;
            }else{
                int8_t val = 0;
                memcpy(&val, ba, 1);
                value = val;
        }   }
        m_ramTable->setItemValue( 3, value  );

        if     ( type.contains( "8" ) ) m_ramTable->setItemValue( 4, decToBase(value, 2, 8)  );
        else if( type.contains( "string" ) )
        {
            QString strVal = "";
            for( int i=address; i<=address+value; i++ )
            {
                QString str = "";
                const QChar cha = getRamValue( i );
                str.setRawData( &cha, 1 );

                strVal += str; //QByteArray::fromHex( getRamValue( i ) );
            }
            m_ramTable->setItemValue( 4, strVal  );
    }   }
    m_ramTable->setItemValue( 2, type  );
}

void DataSpace::addWatchVar( QString name, int address, QString type )
{
    name = name.toUpper();
    //if( !m_regsTable.contains(name) ) m_regList.append( name );
    /// m_regsTable[ name ] = address;
    m_typeTable[ name ] = type;
}
