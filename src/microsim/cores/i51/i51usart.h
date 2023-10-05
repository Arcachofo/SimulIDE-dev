/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51USART_H
#define I51USART_H

#include "mcuuart.h"
#include "mcutypes.h"

class McuTimer;

class MAINMODULE_EXPORT I51Usart : public McuUsart
{
    public:
        I51Usart( eMcu* mcu, QString name, int number );
        ~I51Usart();

        virtual void reset() override;

        virtual void configureA( uint8_t newSCON ) override;
        virtual void configureB( uint8_t newPCON ) override;

        virtual void sendByte( uint8_t data ) override;

        virtual void step();
        virtual uint8_t getBit9();
        virtual void setBit9( uint8_t bit );

    private:
        McuTimer* m_timer1;

        // SCON
        uint8_t*  m_scon;
        regBits_t m_SM;
        regBits_t m_bit9Tx;
        regBits_t m_bit9Rx;

        //PCON
        regBits_t m_SMOD;

        uint8_t m_smodVal;
        bool m_smodDiv;

        bool m_useTimer;
};

#endif

