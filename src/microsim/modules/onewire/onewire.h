/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "e-element.h"

class IoPin;

class OneWire : public eElement
{
    public:
        OneWire( QString id );
        ~OneWire();

        enum w1State_t{
            W1_IDLE=0,
            W1_ROM_CMD,
            W1_FUN_CMD,
            W1_SEARCH,
            W1_MATCH,
            W1_DATA,
            W1_BUSY
        };
        enum w1Command_t {
            W1_SEARCH_ROM = 0xF0,
            W1_SEARCH_ALM = 0xEC,
            W1_READ_ROM   = 0x33,
            W1_MATCH_ROM  = 0x55,
            W1_SKIP_ROM   = 0xCC,
        };

        void stamp() override;
        void voltChanged() override;
        void runEvent() override;

        QString getROM() { return arrayToHex( m_ROM, 8 ); }
        void setROM( QString ROMstr );

    protected:
        virtual void readByte(){;}
        void dataReceived();
        void dataSent();
        void sendData( uint8_t data, int size=8);
        void writeBit();
        void readBit( uint8_t bit );
        void pulse( uint64_t time, uint64_t witdth ); // Time in us

        void sendSearchBit();
        bool bitROM( uint bitIndex );
        void generateROM( uint8_t familyCode );
        uint8_t crc8( uint8_t* addr, uint8_t len );

        virtual void funCommand( uint8_t cmd ){;}
        // ROM commands
        void romCommand( uint8_t cmd );
        void searchROM();       // F0h
        void skipROM();         // CCh
        void readROM();         // 33h
        void matchROM();        // 55h
        void alarmSearch();     // ECh

        w1State_t m_state;
        uint8_t   m_lastCommand;

        bool m_lastIn;
        bool m_write;
        bool m_pullDown;
        bool m_bitROM;
        bool m_parPower;
        bool m_alarm;

        uint64_t m_lastTime;
        uint64_t m_busyTime;
        uint8_t  m_rxReg;
        uint8_t  m_txReg;
        uint8_t  m_lastBit;
        uint8_t  m_bitSearch;
        uint64_t m_bitIndex;

        int m_pulse;
        int m_byte;

        IoPin* m_inpin;

        std::vector<uint8_t> m_txBuff;
        uint8_t m_ROM[8];

 static QString arrayToHex( uint8_t* data, uint len );
};

#endif
