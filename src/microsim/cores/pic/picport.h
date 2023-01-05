/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICPORT_H
#define PICPORT_H

#include "mcuport.h"

class Mcu;
class eMcu;
class McuPin;
class Component;

class MAINMODULE_EXPORT PicPort : public McuPort
{
    public:
        PicPort( eMcu* mcu, QString name );
        ~PicPort();

        virtual void configureA( uint8_t newANSEL ) override;

        virtual void readPort( uint8_t ) override;

    protected:
        virtual McuPin* createPin( int i, QString id , Component* mcu ) override;
};

#endif
