/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRTWI_H
#define AVRTWI_H

#include "mcutwi.h"
//#include "mcutypes.h"

class eMcu;
//class McuPin;

class MAINMODULE_EXPORT AvrTwi : public McuTwi
{
    public:
        AvrTwi( eMcu* mcu, QString name );
        ~AvrTwi();

        virtual void initialize() override;

        virtual void configureA( uint8_t newTWCR ) override;
        virtual void configureB( uint8_t val ) override;

        virtual void writeAddrReg( uint8_t newTWAR ) override;
        virtual void writeStatus( uint8_t newTWSR ) override;
        virtual void writeTwiReg( uint8_t newTWDR ) override;

        virtual void writeByte() override;

    protected:
        virtual void setTwiState( twiState_t state ) override;
        uint8_t getStaus() { return *m_statReg &= 0b11111000; }
        virtual void updateFreq() override;

        uint8_t m_bitRate;

        uint8_t*  m_TWCR;
        //uint8_t*  m_TWSR;

        regBits_t m_TWEN;
        regBits_t m_TWWC;
        regBits_t m_TWSTO;
        regBits_t m_TWSTA;
        regBits_t m_TWEA;
        regBits_t m_TWINT;

};

#endif
