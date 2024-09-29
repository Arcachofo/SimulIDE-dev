/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICUSART_H
#define PICUSART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class PicUsart : public McuUsart
{
    public:
        PicUsart( eMcu* mcu, QString name, int number );
        ~PicUsart();

        virtual void setup() override;

        virtual void configureA( uint8_t newTXSTA ) override;
        virtual void configureB( uint8_t newRCSTA ) override;

        virtual void sendByte( uint8_t data ) override;
        virtual void bufferEmpty() override;
        virtual void frameSent( uint8_t data ) override;

        virtual void setRxFlags( uint16_t frame ) override;

        void setSPBRGL( uint8_t val );
        void setSPBRGH( uint8_t val );
        void setBaurrate( uint8_t val=0 );

        virtual void sleep( int mode ) override;

    private:
        bool m_enabled;

        bool m_speedx2;

        uint8_t*  m_PIR1;
        uint8_t*  m_TXSTA;
        uint8_t*  m_RCSTA;
        uint8_t*  m_SPBRG;
        uint8_t*  m_SPBRGL;
        uint8_t*  m_SPBRGH;

        regBits_t m_txEn;
        regBits_t m_rxEn;

        regBits_t m_TRMT;
        regBits_t m_TXIF;
        regBits_t m_RCIF;
        regBits_t m_BRGH;
        regBits_t m_SPEN;
        regBits_t m_TX9;
        regBits_t m_RX9;
        regBits_t m_ADDEN;
        regBits_t m_FERR;
        regBits_t m_OERR;
        regBits_t m_SYNC;
};

#endif
