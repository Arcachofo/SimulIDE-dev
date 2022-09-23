/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef USARTTX_H
#define USARTTX_H

#include "usartmodule.h"

class MAINMODULE_EXPORT UartTx : public UartTR
{
    public:
        UartTx( UsartModule* usart, eMcu* mcu, QString name );
        ~UartTx();

        virtual void enable( uint8_t en ) override;
        virtual void runEvent() override;
        virtual void processData( uint8_t data ) override;

        void startTransmission();

    protected:
        void sendBit();
};

#endif
