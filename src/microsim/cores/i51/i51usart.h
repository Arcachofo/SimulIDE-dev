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

        virtual void configureA( uint8_t val ) override;
        virtual void step();
        virtual uint8_t getBit9();
        virtual void setBit9( uint8_t bit );

    private:
        McuTimer* m_timer1;
        uint8_t*  m_scon;

        regBits_t m_bit9Tx;
        regBits_t m_bit9Rx;

        //bool m_timerConnected;
        bool m_useTimer;
};

#endif

