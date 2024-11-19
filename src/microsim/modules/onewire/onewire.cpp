/***************************************************************************
 *   Copyright( C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include <QDebug>

#include "onewire.h"
#include "iopin.h"
#include "simulator.h"
#include "circuit.h"
#include "utils.h"

#define tr(str) simulideTr("OneWire",str)

OneWire::OneWire( QString id )
       : eElement( id )
{
    m_state    = W1_IDLE;
    m_inpin = nullptr;
}
OneWire::~OneWire(){}

void OneWire::stamp()   // Called at Simulation Start
{
    m_lastTime = 0;
    m_rxReg   = 0;
    m_lastBit = 7;
    m_write  = false;
    m_lastIn = false;
    m_alarm  = false;

    if( !m_inpin ) return;

    m_inpin->setPinMode( openCo );
    m_inpin->setOutState( true );
    m_inpin->changeCallBack( this, true );
    m_state = W1_IDLE;
}

void OneWire::voltChanged()                              // Called when Input Pin changes
{
    bool inState = m_inpin->getVoltage() > 2.5;
    uint64_t circTime = Simulator::self()->circTime();

    if( m_lastIn && !inState ) m_lastTime = circTime; // Falling edge
    else if( !m_lastIn && inState )                   // Rising edge
    {
        uint64_t time = circTime-m_lastTime; // in picoseconds

        if( time > 205*1e6 )             // > 480 us : Reset (Tested in real device 202 us)
        {
            //qDebug() <<"\n"<< idLabel() << "Ds18b20::voltChanged -------------- RESET"<<time/1e3<<"ns";
            m_rxReg = 0;
            m_bitIndex = 0;
            m_state = W1_ROM_CMD;
            m_write = false;
            pulse( 20, 80 ); // Send 80 us pulse (60 to 240 us)  after 20 us (15 to 60 us)
        }
        else if( m_state > W1_IDLE )    // Active
        {
            if( time > 30*1e6 )         // > 30 us : 0 received
            {
                if( !m_write ) readBit( 0 ); // If writting ignore long pulses
            }
            else if( time < 15*1e6 )          // < 15 us : 1 received Or Read pulse
            {
                if( m_state == W1_BUSY ){ if( m_busyTime > circTime ) pulse( 1, 50 ); } // Still busy: Send a 50 us pulse after 1 us
                else{
                    if( m_write ) writeBit();
                    else          readBit( 1 );
    }   }   }   }
    m_lastIn = inState;
}

void OneWire::runEvent()
{
    if( m_pullDown )   // Pull down
    {
        m_pullDown = false;
        m_inpin->scheduleState( false, 0 );
        Simulator::self()->addEvent( m_pulse, this );
    }
    else{             // Release
        m_inpin->scheduleState( true, 0 );
        m_inpin->changeCallBack( this, true );         // Receive voltChange() CallBacks again
    }
}
void OneWire::writeBit()
{
    if( (m_txReg & 1<<m_bitIndex) == 0 ) pulse( 1, 25 ); // Send a 25 us pulse after 1 us

    if( m_bitIndex == m_lastBit ) // Byte sent
    {
        m_bitIndex = 0;
        m_write = false;
        dataSent();
    }
    else m_bitIndex += 1;
}

void OneWire::dataReceived() // Complete data has been received (it's in m_rxReg)
{
    switch( m_state )
    {
        case W1_IDLE: break;                           // Error, this shoild not happen
        case W1_ROM_CMD: romCommand( m_rxReg ); break; // ROM Command received
        case W1_FUN_CMD: funCommand( m_rxReg ); break; // Function Command received
        case W1_DATA:
        {
            readByte();
        } break;
        case W1_MATCH:          // ROM Match: we are online, wait for Function commands
        {
            m_state = W1_FUN_CMD;
            m_rxReg = 0;
            m_bitIndex = 0;
            m_lastBit = 7;     // Return to normal byte reception
            //qDebug() <<idLabel()<<"Ds18b20::dataReceived     ROM match";
        } break;
        case W1_SEARCH:
        {
            if( ++m_bitSearch == 64 )    // We passed Search ROM: wait for commands
            {
                m_state = W1_IDLE;
                m_lastBit = 7; // Return to normal byte reception
                //qDebug() <<idLabel()<< "Ds18b20::dataReceived  :  Search ROM OK ";
            }
            else if( (m_rxReg > 0) == m_bitROM ) sendSearchBit();   // Bit Match,  keep sending
            else{
                m_state = W1_IDLE; // We are out, Wait for next Reset signal
                //qDebug() <<idLabel()<< "Ds18b20::dataReceived  :  Search ROM OUT";
            }
        } break;
        default: qDebug() << m_elmId << "Ds18b20::dataReceived  :  ERROR";
    }
}

void OneWire::dataSent() // Last data has been sent
{
    //if( m_state != W1_SEARCH ) qDebug() << idLabel() << "Ds18b20::dataSent"<< val2hex( m_txReg );

    if( m_state == W1_SEARCH ) m_lastBit = 0; // Read 1 bit
    else if( !m_txBuff.empty() )   // Send next byte in Tx Buffer
    {
        m_txBuff.pop_back();  // Remove last sent byte
        if( m_txBuff.empty() )// All data sent
        {
            if     ( m_state == W1_ROM_CMD ) m_state = W1_FUN_CMD;  // Wait for Function command
            else if( m_state == W1_FUN_CMD ) m_state = W1_IDLE;     // Wait for Reset pulse
        }
        else sendData( m_txBuff.back() ); // Send last byte in list, if list is not empty
    }
}

void OneWire::readBit( uint8_t bit )
{
    if( m_state == W1_MATCH )  // Compare bit received with ROM
    {
        if( (bit>0) != bitROM( m_bitIndex ) )
        {
            m_state = W1_IDLE; // No ROM match, we are out
            m_rxReg = 0;
            m_bitIndex = 0;
            m_lastBit = 7;     // Return to normal byte reception
            //qDebug() <<idLabel()<<"Ds18b20::readBit       NO ROM match";
            return;
        }
    }
    if( bit ) m_rxReg |= 1<<m_bitIndex;

    if( m_bitIndex == m_lastBit )   // Complete byte received
    {
        m_bitIndex = 0;
        m_lastBit = 7;   // Return to "byte" mode
        dataReceived();
        m_rxReg = 0;
    }
    else m_bitIndex += 1;
}

void OneWire::pulse( uint64_t time, uint64_t witdth ) // Time in us
{
    m_pullDown = true;
    m_pulse = witdth*1e6;                          // Keep line low for width us
    m_inpin->changeCallBack( this, false );          // Stop receiving voltChange() CallBacks
    Simulator::self()->addEvent( time*1e6, this ); // Send pulse after time us
}

void OneWire::sendData( uint8_t data, int size )
{
    m_txReg = data;
    m_bitIndex = 0;
    m_lastBit = size-1;
    m_write = true;
}


// ROM COMMANDS --------------------------------------------------

void OneWire::romCommand( uint8_t cmd )
{
    switch( cmd )
    {
        case 0x33: readROM();   break;
        case 0x55: matchROM();  break;
        case 0xCC: skipROM();   break;
        case 0xF0: searchROM(); break;
        default:{
            m_state = W1_IDLE;
            qDebug()<< m_elmId << "OneWire::command : Warning: ROM command Not implemented";
        }
    }
    m_lastCommand = cmd;
}

void OneWire::readROM() // Code: 33h : send ROM to Master
{
    //qDebug() << idLabel() <<"OneWire::readROM"<< arrayToHex( m_ROM, 8 );

    m_txBuff.clear();
    for( int i=7; i>=0; i-- ) m_txBuff.push_back( m_ROM[i] );

    sendData( m_txBuff.back() );
}

void OneWire::matchROM() // Code 55h : read 64 bits and compare with ROM
{
    //qDebug() <<idLabel()<< "OneWire::matchROM";
    m_lastBit = 63;
    m_state = W1_MATCH;
}

void OneWire::skipROM() // Code: CCh : This device is selected, Wait command
{
    //qDebug() <<idLabel()<< "OneWire::skipROM";
    m_state = W1_FUN_CMD;
}

void OneWire::searchROM() // Code F0h
{
    //qDebug() <<idLabel()<< "OneWire::searchROM";
    m_state = W1_SEARCH;
    m_bitSearch = 0;
    sendSearchBit();
}

void OneWire::alarmSearch()
{
    if( m_alarm ) searchROM();
    else          m_state = W1_IDLE;
}

void OneWire::sendSearchBit()
{
    m_bitROM = bitROM( m_bitSearch );
    uint8_t data = m_bitROM ? 1 : 2;
    sendData( data, 2 );
}

bool OneWire::bitROM( uint bitIndex )
{
    return ( m_ROM[ bitIndex/8 ] & 1<<(bitIndex%8) ) > 0;
}

uint8_t OneWire::crc8( uint8_t* addr, uint8_t len ) // OneWire crc8 calc
{
    uint8_t crc = 0;

    while( len--)
    {
        uint8_t inbyte = *addr++;
        for( uint8_t i=8; i; i-- )
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if( mix) crc ^= 0x8C;
            inbyte >>= 1;
    }   }
    return crc;
}

void OneWire::generateROM( uint8_t familyCode ) // Generate unique ROM address
{
  // 8-bit CRC | 48 bit S/N | 8-BIT FAMILY CODE, MSB - LSB
  m_ROM[0] = familyCode;
  m_ROM[1] = rand();
  m_ROM[2] = rand();
  m_ROM[3] = rand();
  m_ROM[4] = rand();
  m_ROM[5] = 0x00;
  m_ROM[6] = 0x00;
  m_ROM[7] = crc8( m_ROM, 7 );

  //qDebug() << idLabel() <<"OneWire::generateROM"<<arrayToHex( m_ROM, 8 );
}

void OneWire::setROM( QString ROMstr )
{
    if( Circuit::self()->pasting() ) return;

    bool ok;
    QStringList lstROM = ROMstr.split(" ");
    lstROM.removeAll("");
    for( int i=0; i<8; ++i ) m_ROM[i] = lstROM.at( i ).toInt( &ok, 16 );
    //qDebug() << idLabel() <<"OneWire::setROM"<<arrayToHex( m_ROM, 8 );
}

QString OneWire::arrayToHex( uint8_t* data, uint len ) // Static
{
    QString hexValue;
    for( uint i=0; i<len; i++) hexValue.append( val2hex( data[i] ) ).append(" ");
    return hexValue;
}
