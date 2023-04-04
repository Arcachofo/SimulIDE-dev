/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICVREF_H
#define PICVREF_H

#include "mcuvref.h"

class MAINMODULE_EXPORT PicVref : public McuVref
{
    public:
        PicVref( eMcu* mcu, QString name );
        ~PicVref();

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

#endif
