/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "avreeprom.h"
#include "datautils.h"
#include "e_mcu.h"
#include "simulator.h"

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
    bool eempe = getRegBitsBool( *m_EECR, m_EEMPE );
    if( eempe ) clearRegBits( m_EEMPE );            // No read operation took place: clear EEMPE
    else        clearRegBits( m_EEPE );             // Read operation took place: clear EEPE
}

void AvrEeprom::configureA( uint8_t newEECR ) // EECR is being written
{
    bool eempe = getRegBitsBool( newEECR, m_EEMPE );
    bool eepe  = getRegBitsBool( newEECR, m_EEPE );

    if( !eepe )
    {
        m_mode = getRegBitsVal( newEECR, m_EEPM ); // EEPROM Programming Mode Bits

        if( getRegBitsBool( newEECR, m_EERE ) ) // Read triggered
        {
            m_mcu->m_regOverride = newEECR & ~(m_EERE.mask); // Clear EERE: it happens after 4 cycles, but cpu is halted for these cycles
            m_mcu->cyclesDone += 4;
            readEeprom(); // Should we return here?
        }
    }
    if( eempe )
    {
        bool oldEepe = getRegBitsBool( *m_EECR, m_EEPE );

        if( !oldEepe && eepe ) // Write triggered
        {
            m_mcu->m_regOverride = newEECR & ~(m_EEMPE.mask); // Clear EEMPE: it happens after 4 cycles but we need to cancel it now
            Simulator::self()->cancelEvents( this );          // Cancel EEMPE clear event
            m_mcu->cyclesDone += 2;
            writeEeprom();
            return;
        }
        bool oldEempe = getRegBitsBool( *m_EECR, m_EEMPE );
        if( !oldEempe ) // Shedule EEMPE clear: 4 cycles
            Simulator::self()->addEvent( m_mcu->psCycle()*4, this );
    }
}

void AvrEeprom::writeEeprom()
{
    uint8_t data = *m_dataReg;
    uint64_t time;

    if( m_EEPM.mask ) // Has EEPM bits: atmega328 and friends
    {
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
    }
    else            // No EEPM bits: atmega8 8.5 ms
    {
        time = 8500*1e6; // picoseconds
    }
    m_mcu->setRomValue( m_address, data );
    Simulator::self()->addEvent( time, this ); // Shedule Write cycle end
}

