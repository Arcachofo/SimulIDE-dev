/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avreeprom.h"
#include "datautils.h"
#include "e_mcu.h"
#include "simulator.h"

AvrEeprom::AvrEeprom( eMcu* mcu, QString name )
         : McuEeprom( mcu, name )
{
    m_EECR  = mcu->getReg( "EECR" );
    m_EEPM  = getRegBits( "EEPM0, EEPM1", mcu );
    m_EEMPE = getRegBits( "EEMPE", mcu );
    m_EEPE  = getRegBits( "EEPE", mcu );
    m_EERE  = getRegBits( "EERE", mcu );
}
AvrEeprom::~AvrEeprom(){}

void AvrEeprom::initialize()
{
    m_mode = 0;
    McuEeprom::initialize();
}

void AvrEeprom::runEvent() // Write cycle end reached
{
    clearRegBits( m_EEPE );
}

void AvrEeprom::configureA( uint8_t newEECR ) // EECR is being written
{
    m_mode = getRegBitsVal( newEECR, m_EEPM );

    bool eempe = getRegBitsBool( newEECR, m_EEMPE );
    bool eepe  = getRegBitsBool( newEECR, m_EEPE );

    if( eempe )
    {
        bool oldEepe = getRegBitsBool( *m_EECR, m_EEPE );

        if( !oldEepe && eepe ) // Write triggered
        {
            if( m_mcu->cycle() <= m_nextCycle ) writeEeprom();// write data
            return;
            /// else clearRegBits( m_EEPE );
        }
        bool oldEempe = getRegBitsBool( *m_EECR, m_EEMPE );
        if( !oldEempe ) // Set maximun cycle to procedd to write
        {
            m_nextCycle = m_mcu->cycle()+4;
        }
    }
    if( !eepe && getRegBitsBool( newEECR, m_EERE ) ) // Read enable
    {
        m_mcu->cyclesDone += 4;
        readEeprom();
    }
}

void AvrEeprom::writeEeprom()
{
    uint8_t data = *m_dataReg;
    uint64_t time;

    switch( m_mode )
    {
        case 0:     // 3.4 ms - Erase and Write in one operation (Atomic Operation)
            time = 3400*1e6; // picoseconds
            break;
        case 1:     // 1.8 ms - Erase Only
            data = 0xFF;
            // fallthrough
        case 2:     // 1.8 ms - Write Only
            time = 1800*1e6; // picoseconds
            break;
    }
    m_mcu->setRomValue( m_address, data );
    m_mcu->cyclesDone += 2;

    Simulator::self()->addEvent( time, this ); // Shedule Write cycle end
}

