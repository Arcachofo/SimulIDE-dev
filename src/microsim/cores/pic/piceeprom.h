/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICEEPROM_H
#define PICEEPROM_H

#include "mcueeprom.h"

class MAINMODULE_EXPORT PicEeprom : public McuEeprom
{
        friend class McuCreator;

    public:
        PicEeprom( eMcu* mcu, QString name );
        ~PicEeprom();

        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void configureA( uint8_t newEECON1 ) override;
        virtual void configureB( uint8_t newEECON2 ) override;

    private:
        uint64_t m_nextCycle;
        bool m_writeEnable;

        uint8_t m_wrMask;

        regBits_t m_WRERR;
        regBits_t m_WREN;
        regBits_t m_WR;
        regBits_t m_RD;
};

#endif
