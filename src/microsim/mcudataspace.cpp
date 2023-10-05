/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "mcudataspace.h"
#include "datautils.h"
#include "utils.h"

DataSpace::DataSpace()
{
    m_sregAddr = 0;
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
    for( uint i=0; i<m_dataMem.size(); i++ ) writeReg( i, 0, false );

    for( QString regName : m_regInfo.keys() )  // Set Registers Reset Values
    {
        regInfo_t regInfo = m_regInfo.value(regName);
        if( regInfo.resetVal != 0 )
        {
            writeReg( regInfo.address, regInfo.resetVal, false );
            m_dataMem[regInfo.address] = regInfo.resetVal;
        }
    }
}

uint8_t DataSpace::readReg( uint16_t addr )
{
    uint8_t v = m_dataMem[addr];
    regSignal_t* regSignal = m_regSignals.value( addr );
    if( regSignal )
    {
        m_regOverride = -1;
        regSignal->on_read.emitValue( v );
        if( m_regOverride >= 0 ) v = (uint8_t)m_regOverride; // Value overriden in callback
        else                     v = m_dataMem[addr];        // Timers update their counters in callback
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

