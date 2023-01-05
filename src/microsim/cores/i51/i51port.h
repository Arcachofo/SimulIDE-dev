/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51PORT_H
#define I51PORT_H

#include "mcuport.h"


class MAINMODULE_EXPORT I51Port : public McuPort
{
    public:
        I51Port( eMcu* mcu, QString name );
        ~I51Port();

        virtual void reset() override;

        virtual void readPort( uint8_t ) override;

    protected:
        virtual McuPin* createPin( int i, QString id , Component* mcu ) override;
};

#endif
