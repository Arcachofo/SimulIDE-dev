/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "extmem.h"
#include "e_mcu.h"
#include "simulator.h"

ExtMemModule::ExtMemModule( eMcu* mcu, QString name )
            : McuModule( mcu, name )
            , eElement( mcu->getId()+"-"+name )
{
    m_rwPin = NULL;
    m_rePin = NULL;
    m_enPin = NULL;
    m_laPin = NULL;

    m_laEnSetTime  = 0;
    m_addrSetTime  = 0; //250*1e9;
    m_laEnEndTime  = 0;
    m_readSetTime  = 0; //750*1e9;
    m_writeSetTime = 0; //600*1e9;
    m_readBusTime  = 0; //800*1e9;
}
ExtMemModule::~ExtMemModule() {}

//void ExtMemModule::initialize(){;}

void ExtMemModule::reset()  // NO: Reset happens after initialize() in Pins.
{
    Simulator::self()->cancelEvents( this );
    m_memState = mem_IDLE;
    m_read = true;

    if( m_rwPin ){
        m_rwPin->controlPin( true, true );
        m_rwPin->setPinMode( output );
        m_rwPin->setOutState( true );
    }
    if( m_rePin )
        m_rePin->sheduleState( true, 0 );

    if( m_enPin ){
        m_enPin->controlPin( true, true );
        m_enPin->setPinMode( output );
        m_enPin->setOutState( true );
        m_enPin->updateStep();
    }
    if( m_laPin ){
        m_laPin->controlPin( true, true );
        m_laPin->setPinMode( output );
        m_laPin->setOutState( false );
        m_laPin->updateStep();
    }

    for( McuPin* pin : m_addrPin )
    {
        pin->setDirection( true );
        pin->controlPin( true, true );
        pin->setOutState( false );
        pin->updateStep();
    }
    for( McuPin* pin : m_dataPin )
    {
        pin->setDirection( false );
        pin->controlPin( true, true );
        pin->updateStep();
    }
}

void ExtMemModule::runEvent()
{
    switch( m_memState ) {
        case mem_IDLE: break;
        case mem_LAEN:            // Enable Latch for Low addr
        {
            m_laPin->sheduleState( true, 0 );
            uint64_t time = m_addrSetTime - m_laEnSetTime;
            Simulator::self()->addEvent( time, this );
            m_memState = mem_ADDR;
        }break;
        case mem_ADDR:            // Set Address Bus
        {
            //if( m_readMode & RW ) m_rwPin->sheduleState( m_read, 0 ); // Set RW Pin Hi/Lo

            uint64_t time;
            uint addr = m_addr;

            if( m_readMode & LA ) // We are latching Low Address byte in Data Bus
            {
                setDataDir( output );
                setDataBus( m_addr );         // Low byte addr to Data Pins
                addr = m_addrH;               // Addr Pins for High byte addr
                time = m_laEnEndTime - m_addrSetTime;
                m_memState = mem_LADI;
            }else{
                m_memState = mem_DATA;
                m_dataTime = m_read ? m_readSetTime : m_writeSetTime;
                time = m_dataTime -m_addrSetTime;
            }
            setAddrBus( addr );               // Set addr Pins states

            Simulator::self()->addEvent( time, this );
        } break;
        case mem_LADI:           // Disable Latch (if in use)
        {
            m_laPin->sheduleState( false, 0 );
            m_memState = mem_DATA;

            m_dataTime = m_read ? m_readSetTime : m_writeSetTime;
            uint64_t time = m_dataTime -m_laEnEndTime;
            Simulator::self()->addEvent( time, this );
        }break;
        case mem_DATA:           // Set Data Bus for Read or Write
        {
            setDataDir( m_read ? input : output ); // Set data  Pins In/Out
            if( m_readMode & RW ) m_rwPin->setOutState( m_read ); // Set RW Pin Hi/Lo

            if( m_read )
            {
                if( m_readMode & EN ) m_enPin->sheduleState( false, 1 ); // Set EN  Pin Low
                uint64_t time = m_readBusTime - m_dataTime;
                Simulator::self()->addEvent( time, this );
            }
            else setDataBus( m_data ); // Set data Pins States

            m_memState = m_read ? mem_READ : mem_IDLE;
        } break;
        case mem_READ:           // Read Data Bus
        {
            m_data = 0;
            for( uint i=0; i<m_dataPin.size(); ++i )
            {
                bool state = m_dataPin[i]->getInpState();
                if( state ) m_data += pow( 2, i );
            }
            m_memState = mem_IDLE;
            if( m_readMode & EN ) m_enPin->sheduleState( true, 1 ); // Set EN  Pin High
        } break;
    }
}

void ExtMemModule::voltChanged()
{;}

void ExtMemModule::read( uint32_t addr, uint8_t mode )
{
    m_read = true;
    m_addr = addr;
    m_readMode = mode;
    m_dataTime = 0;

    if( mode & LA ){
        m_addrH = addr >> 8;
        m_memState = mem_LAEN;
        Simulator::self()->addEvent( m_laEnSetTime, this );
    }else{
        if( m_memState != mem_IDLE )
            qDebug() << "ERROR: ExtMemModule::read Operation not finished";
        m_memState = mem_ADDR;
        Simulator::self()->addEvent( m_addrSetTime, this );
    }
}

void ExtMemModule::write( uint32_t addr, uint32_t data )
{
    m_read = false;
    m_addr = addr;
    m_data = data;
    m_memState = mem_ADDR;
    Simulator::self()->addEvent( m_addrSetTime, this );
}

void ExtMemModule::setAddrBus( uint32_t addr )
{
    for( IoPin* pin : m_addrPin )     // Set addr Pins states
    {
        pin->sheduleState( addr & 1, 0 );
        addr >>= 1;
    }
}

void ExtMemModule::setDataDir( pinMode_t dir ) // Set data Pins In/Out
{
    for( IoPin* pin : m_dataPin ) pin->setPinMode( dir );
}

void ExtMemModule::setDataBus( uint32_t data )
{
    for( IoPin* pin : m_dataPin ){
            pin->sheduleState( data & 1, 0 );
            data >>= 1;
    }
}
