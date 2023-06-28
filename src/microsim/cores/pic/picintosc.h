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

        virtual void setPin( int n, McuPin* p ) override;

 static PicIntOsc* createIntOsc( eMcu* mcu, QString name, QString type );

    protected:
        bool m_cfgWordCtrl;
};

class PicIntOsc00 : public PicIntOsc
{
    public:
        PicIntOsc00( eMcu* mcu, QString name );
        ~PicIntOsc00();

        virtual void configureA( uint8_t newOSCCON ) override;

    protected:
        regBits_t m_SCS;
        regBits_t m_IRCF;
};

class PicIntOsc01 : public PicIntOsc
{
    public:
        PicIntOsc01( eMcu* mcu, QString name );
        ~PicIntOsc01();

        virtual void configureA( uint8_t newOSCCON ) override;

    protected:
        regBits_t m_OSCF;
};

class PicIntOsc02 : public PicIntOsc
{
    public:
        PicIntOsc02( eMcu* mcu, QString name );
        ~PicIntOsc02();

        virtual void configureA( uint8_t newOSCCON ) override;

    protected:
        regBits_t m_SCS;
        regBits_t m_IRCF;
};
#endif
