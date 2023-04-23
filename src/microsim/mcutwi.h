/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUTWI_H
#define MCUTWI_H

#include "mcuprescaled.h"
#include "twimodule.h"

class eMcu;
class McuPin;

class MAINMODULE_EXPORT McuTwi : public McuPrescaled, public TwiModule
{
        friend class McuCreator;

    public:
        McuTwi( eMcu* mcu, QString name );
        ~McuTwi();

        virtual void initialize() override;

        virtual void writeAddrReg( uint8_t val ){ m_address = val; }
        virtual void writeStatus( uint8_t val ){;}
        virtual void writeTwiReg( uint8_t val ){;}
        virtual void readTwiReg( uint8_t val ){;}

    protected:
        virtual void updateFreq() {;}

        uint8_t* m_addrReg;
        uint8_t* m_dataReg;
        uint8_t* m_statReg;
};

#endif
