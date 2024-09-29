/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICVREF_H
#define PICVREF_H

#include "mcuvref.h"

class PicVref : public McuVref
{
    public:
        PicVref( eMcu* mcu, QString name );
        ~PicVref();

        virtual void setup() override;
        virtual void initialize() override;

        virtual void configureA( uint8_t newVRCON ) override;

    protected:
        //virtual void setMode( uint8_t mode ) override;

        bool m_vrr;
        bool m_vroe;

        regBits_t m_VREN;
        regBits_t m_VROE;
        regBits_t m_VRR;
        regBits_t m_VR;
};

class PicVrefE : public McuVref
{
    public:
        PicVrefE( eMcu* mcu, QString name );
        ~PicVrefE();

        virtual void setup() override;

        virtual void configureA( uint8_t newFVRCON ) override;

        double getAdcVref();
        double getDacVref();

    private:
        double m_adcVref;
        double m_dacVref;

        regBits_t m_FVREN;
        regBits_t m_CDAFVR;
        regBits_t m_ADFVR;
};

#endif
