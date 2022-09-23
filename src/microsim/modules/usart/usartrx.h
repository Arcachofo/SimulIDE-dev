/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef USARTRX_H
#define USARTRX_H

#include <queue>

#include "usartmodule.h"


class MAINMODULE_EXPORT UartRx : public UartTR
{
    public:
        UartRx( UsartModule* usart, eMcu* mcu, QString name );
        ~UartRx();

        enum error{
            frameError =1<<12,
            dataOverrun=1<<13,
            parityError=1<<14
        };

        virtual void enable( uint8_t en ) override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        virtual void processData( uint8_t ) override;
        virtual uint8_t getData() override;

        void queueData( uint8_t data );

    protected:
        void readBit();
        void rxEnd();
        void byteReceived( uint16_t frame );

        bool m_startHigh;

        uint16_t m_fifo[2];
        uint8_t  m_fifoP;

        std::queue<uint8_t> m_inBuffer;
};

#endif
