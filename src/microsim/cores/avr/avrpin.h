/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRPIN_H
#define AVRPIN_H

#include "mcupin.h"

class Component;
class McuPort;
class eMcu;

class MAINMODULE_EXPORT AvrPin : public McuPin
{
    friend class McuPort;

    public:
        AvrPin( McuPort* port, int i, QString id , Component* mcu );
        ~AvrPin();

        virtual void setPortState( bool state ) override;

    protected:

};

#endif
