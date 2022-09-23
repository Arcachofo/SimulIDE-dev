/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVROCUNIT_H
#define AVROCUNIT_H

#include "mcuocunit.h"

class MAINMODULE_EXPORT AvrOcUnit : public McuOcUnit
{
        friend class McuCreator;

    public:
        AvrOcUnit( eMcu* mcu, QString name );
        ~AvrOcUnit();

        virtual void configure( uint8_t val ) override;

    protected:
        virtual void setPinSate( bool state );
};

#endif
