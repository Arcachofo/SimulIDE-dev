/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRSLEEP_H
#define AVRSLEEP_H

#include "mcusleep.h"
#include "mcutypes.h"

enum sleepMode_t{
    sleepIdle=0,
    sleepAdcNR,
    sleepPowDo,
    sleepPowSa,
    sleepRes0,
    sleepRes1,
    sleepStand,
    sleepExtSt,
    sleepNone
};

class MAINMODULE_EXPORT AvrSleep : public McuSleep
{
    public:
        AvrSleep( eMcu* mcu, QString name );
        ~AvrSleep();

        virtual void initialize() override;
        virtual void configureA( uint8_t newVal ) override;

        //virtual void sleep() override;

    protected:
        //sleepMode_t m_sleepMode;

        regBits_t m_SM;
        regBits_t m_SE;

        std::vector<Interrupt*> m_wakeUps;
};

/*class MAINMODULE_EXPORT AvrSleep00 : public AvrSleep
{
    public:
        AvrSleep00( eMcu* mcu, QString name );
        ~AvrSleep00();
}*/
#endif
