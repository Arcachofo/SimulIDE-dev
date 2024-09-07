/***************************************************************************
 *   Copyright (C) 2024 by Coethium                                        *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRINTOSC_H
#define AVRINTOSC_H

#include "mcuintosc.h"

class AvrIntOsc : public McuIntOsc
{
    public:
        AvrIntOsc(eMcu* mcu, QString name );
        virtual ~AvrIntOsc();

        virtual void reset() override;

        virtual void configureA(uint8_t newCLKPR) override;

        virtual bool freqChanged() override;

    private:
        uint8_t m_prIndex;

        regBits_t m_CLKPS;
};

#endif
