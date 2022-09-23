/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51PIN_H
#define I51PIN_H

#include "mcupin.h"

class Component;
class McuPort;
class eMcu;

class MAINMODULE_EXPORT I51Pin : public McuPin
{
    friend class McuPort;

    public:
        I51Pin( McuPort* port, int i, QString id , Component* mcu );
        ~I51Pin();

        //virtual void initialize() override;

        virtual void setOutState( bool state ) override;

        virtual void ConfExtInt( uint8_t bits ) override;

    protected:

};

#endif
