/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUUART_H
#define MCUUART_H

#include "usartmodule.h"
#include "mcumodule.h"

class MAINMODULE_EXPORT McuUsart : public McuModule, public UsartModule
{
        friend class McuCreator;

    public:
        McuUsart( eMcu* mcu, QString name, int number );
        virtual ~McuUsart();

        virtual void sendByte( uint8_t data ) override{ UsartModule::sendByte( data ); }
        virtual void bufferEmpty() override;
        virtual void frameSent( uint8_t data ) override;
        virtual void readByte( uint8_t data ) override;

    protected:
        int m_number;

        bool m_speedx2;
};

#endif
