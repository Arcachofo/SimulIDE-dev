/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRUSART_H
#define AVRUSART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class AvrUsart : public McuUsart
{
    public:
        AvrUsart( eMcu* mcu, QString name, int number );
        ~AvrUsart();

        virtual void setup() override;

        virtual void configureA( uint8_t newUCSRnA ) override;
        virtual void configureB( uint8_t newUCSRnB ) override;
        virtual void configureC( uint8_t newUCSRnC ) override;

        virtual void sendByte( uint8_t data ) override;
        virtual void frameSent( uint8_t data ) override;

        virtual void setRxFlags( uint16_t frame ) override;

        void setBaurrate( uint8_t ubrr=0 );

    private:
        void setUBRRnL( uint8_t v );
        void setUBRRnH( uint8_t v );

        uint8_t*  m_UCSRnA;
        //uint8_t*  m_UCSRnB;
        uint8_t*  m_UBRRnL;
        uint8_t*  m_UBRRnH;
        uint8_t  m_UBRRHval;

        uint8_t m_ucsz01;
        uint8_t m_ucsz2;

        regBits_t m_txEn;
        regBits_t m_rxEn;

        regBits_t m_modeRB;
        regBits_t m_pariRB;
        regBits_t m_stopRB;
        regBits_t m_UCSZ01;
        regBits_t m_UCSZ2;
        regBits_t m_u2xn;

        regBits_t m_UDRIE;
        regBits_t m_UDRE;
        regBits_t m_TXC;
        regBits_t m_RXC;
        regBits_t m_FE;
        regBits_t m_DOR;
        regBits_t m_UPE;
        regBits_t m_MPCM;
};

#endif
