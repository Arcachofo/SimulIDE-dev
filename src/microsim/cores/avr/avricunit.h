/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRICUNIT_H
#define AVRICUNIT_H

#include "mcuicunit.h"

class AvrIcUnit : public McuIcUnit
{
    public:
        AvrIcUnit( eMcu* mcu, QString name );
        virtual ~AvrIcUnit();

        virtual void configureA( uint8_t val ) override;
};

#endif
