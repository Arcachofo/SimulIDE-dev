/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICINTOSC_H
#define PICINTOSC_H

#include "mcuintosc.h"

class PicIntOsc : public McuIntOsc
{
    public:
        PicIntOsc( eMcu* mcu, QString name );
        ~PicIntOsc();

        virtual void stamp() override;

        virtual void configureA( uint8_t newOSCCON ) override;

    protected:
        bool m_cfgWordCtrl;

        regBits_t m_SCS;
        regBits_t m_IRCF;
};
#endif
