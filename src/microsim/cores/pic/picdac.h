/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/


#ifndef PICDAC_H
#define PICDAC_H

#include "mcudac.h"

class eMcu;
class McuPin;
class PicVrefE;

class PicDac:  public McuDac
{
    public:
        PicDac( eMcu* mcu, QString name );
        ~PicDac();

        virtual void setup() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configureA( uint8_t newDACCON0 ) override;

        virtual void outRegChanged( uint8_t val ) override;

        virtual void callBack() override;

    protected:
        void updtOutVolt();

        bool m_useFVR;
        bool m_usePinP;
        bool m_usePinN;

        // DACCON0
        regBits_t m_DACEN;
        regBits_t m_DACLPS;
        regBits_t m_DACOE;
        regBits_t m_DACPSS;
        regBits_t m_DACNSS;

        // DACCON1
        regBits_t m_DACR;

        uint8_t m_daclps;

        PicVrefE* m_fvr;
};

#endif
