/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef USARTMODULE_H
#define USARTMODULE_H

#include<QList>

#include "mcumodule.h"
#include "e-element.h"

#define mSTOPBITS m_usart->m_stopBits
#define mDATABITS m_usart->m_dataBits
#define mDATAMASK m_usart->m_dataMask
#define mPARITY   m_usart->m_parity

enum parity_t{
    parNONE=0,
    parEVEN=2,
    parODD=3,
};

enum rxError_t{
    frameError =1<<12,
    dataOverrun=1<<13,
    parityError=1<<14
};

class IoPin;
class UartTx;
class UartRx;
class UartSync;
class SerialMonitor;

class UsartModule
{
        friend class eMcu;
    public:
        UsartModule( eMcu* mcu, QString name );
        ~UsartModule();

        int  baudRate() { return m_baudRate; }
        void setBaudRate( int br );
        void setDataBits( uint8_t b );

        bool serialMon() { return m_serialMon; }

        virtual uint8_t getBit9Tx(){return 0;}
        virtual void setBit9Rx( uint8_t bit ){;}

        virtual void sendByte( uint8_t data );
        virtual void bufferEmpty(){;}
        virtual void frameSent( uint8_t data );
        virtual void readByte( uint8_t data ){;}
        virtual void byteReceived( uint8_t data );
        virtual void setRxFlags( uint16_t frame ){;}

        void openMonitor( QString id, int num=0, bool send=false );
        virtual void monitorClosed();

        uint8_t m_mode;
        uint8_t m_stopBits;
        uint8_t m_dataBits;
        uint8_t m_dataMask;
        parity_t m_parity;

    protected:
        void setPeriod( uint64_t period );
        void setSynchronous( bool s );

        bool m_synchronous;
        bool m_serialMon;

        int m_baudRate;

        SerialMonitor* m_monitor;

        UartSync* m_uartSync;
        UartTx* m_sender;
        UartRx* m_receiver;
};

class UartSync : public eElement
{
    friend class UsartModule;

    public:
        UartSync( UsartModule* usart, /*eMcu* mcu,*/ QString name );
        ~UartSync();

        virtual void runEvent() override;

        void setClkOffset( uint64_t o ) { m_syncClkOffset = o; }
        void sendSyncData( uint8_t data );
        void setPeriod( uint64_t p ) { m_syncPeriod = p; }

    private:
        int m_currentBit;
        bool m_clkState;

        uint8_t  m_syncData;
        uint16_t m_frame;
        uint64_t m_syncPeriod;
        uint64_t m_syncClkOffset; // Synchronous clock first edge after data set

        UsartModule* m_usart;

        IoPin* m_syncTxPin;
        IoPin* m_syncClkPin;
};

class Interrupt;

class UartTR : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        UartTR( UsartModule* usart, eMcu* mcu, QString name );
        ~UartTR();

        enum state_t{
            usartSTOPPED=0,
            usartIDLE,
            usartTRANSMIT,
            usartCLOCK,
            usartTXEND,
            usartRECEIVE,
        };

        virtual void initialize() override;

        virtual void enable( uint8_t ){;}
        virtual uint8_t getData() { return  m_data; }

        virtual void configureA( uint8_t val ) override;

        bool isEnabled() { return m_enabled; }

        void setPeriod( uint64_t p ) { m_period = p; }
        bool getParity( uint16_t data );

        state_t state() { return m_state; }

        void setPins( QList<IoPin*> pinList );
        IoPin* getPin() { return m_ioPin; }

        void raiseInt( uint8_t data=0 );

    protected:
        UsartModule* m_usart;
        IoPin* m_ioPin;
        QList<IoPin*> m_pinList;

        uint8_t m_buffer;
        uint8_t m_data;
        uint16_t m_frame;
        uint8_t m_framesize;
        uint8_t m_currentBit;
        uint8_t m_bit9;
        state_t m_state;

        bool m_enabled;

        uint64_t m_period; // Baudrate period
};

#endif
