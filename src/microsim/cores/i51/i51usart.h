/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51USART_H
#define I51USART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class I51Usart : public McuUsart
{
    public:
        I51Usart( eMcu* mcu, QString name, int number );
        ~I51Usart();

        virtual void setup() override;
        virtual void reset() override;

        virtual void configureA( uint8_t newSCON ) override;
        virtual void configureB( uint8_t newPCON ) override;

        virtual void sendByte( uint8_t data ) override;
        virtual void readByte( uint8_t data ) override;
        virtual void setRxFlags( uint16_t frame ) override;

        virtual void callBack() override; // Called by Timer 1 interrupt

    private:
        McuTimer* m_timer1;

        // SCON
        uint8_t*  m_scon;
        regBits_t m_SM;
        regBits_t m_SM2;

        //PCON
        regBits_t m_SMOD;

        int m_counter;

        uint8_t m_smodVal;
        bool m_smodDiv;

        bool m_stopBitError;
};

#endif

