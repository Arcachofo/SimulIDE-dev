/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICMSSP_H
#define PICMSSP_H

#include "mcumodule.h"
#include "e-element.h"
#include "mcutypes.h"

class PicSpi;
class PicTwi;
//class McuPin;

enum sspMode_t{
    sspOFF=0,
    sspSPI_M,
    sspSPI_S,
    sspI2C_M,
    sspI2C_S
};

class MAINMODULE_EXPORT PicMssp : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        PicMssp( eMcu* mcu, QString name, int type );
        ~PicMssp();

        virtual void initialize();

        virtual void configureA( uint8_t SSPCON ) override;

        //virtual void setInterrupt( Interrupt* i ) override;

        //void setPin( McuPin* pin );

    protected:
        uint8_t m_mode;
        sspMode_t m_sspMode;

        /*uint8_t* m_ccpRegL;
        uint8_t* m_ccpRegH;*/

        regBits_t m_SSPMx;

        PicSpi*  m_spiUnit;
        PicTwi*  m_twiUnit;
};

#endif
