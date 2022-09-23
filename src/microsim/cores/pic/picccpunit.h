/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICCCPUNIT_H
#define PICCCPUNIT_H

#include "mcumodule.h"
#include "e-element.h"

class PicOcUnit;
class PicIcUnit;
class PicPwmUnit;
class McuPin;

enum ccpMode_t{
    ccpOFF=0,
    ccpCAP,
    ccpCOM,
    ccpPWM,
};

class MAINMODULE_EXPORT PicCcpUnit : public McuModule, public eElement
{
    friend class McuCreator;

    public:
        PicCcpUnit( eMcu* mcu, QString name, int type );
        ~PicCcpUnit();

        virtual void initialize();

        virtual void ccprWriteL( uint8_t val );
        virtual void ccprWriteH( uint8_t val );

        virtual void configureA( uint8_t CCPxCON ) override;

        virtual void setInterrupt( Interrupt* i ) override;

        void setPin( McuPin* pin );

    protected:
        uint8_t m_mode;
        ccpMode_t m_ccpMode;

        uint8_t* m_ccpRegL;
        uint8_t* m_ccpRegH;

        regBits_t m_CCPxM;

        PicIcUnit*  m_capUnit;
        PicOcUnit*  m_comUnit;
        PicPwmUnit* m_pwmUnit;
};

#endif
