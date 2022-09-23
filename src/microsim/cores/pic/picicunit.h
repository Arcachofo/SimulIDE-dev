/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICICUNIT_H
#define PICICUNIT_H

#include "mcuicunit.h"


class MAINMODULE_EXPORT PicIcUnit : public McuIcUnit
{
    friend class PicCcpUnit;

    public:
        PicIcUnit( eMcu* mcu, QString name );
        virtual ~PicIcUnit();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configure( uint8_t CCPxM ) override;

    private:
        uint64_t m_prescaler;
        uint64_t m_counter;
};

#endif
