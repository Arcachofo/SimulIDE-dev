/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICWDT_H
#define PICWDT_H

#include "mcuwdt.h"

class PicWdt : public McuWdt
{
    public:
        PicWdt( eMcu* mcu, QString name );
        ~PicWdt();

        virtual void setup() override;
        virtual void initialize() override;
        virtual void runEvent() override;

        virtual void configureA( uint8_t newOPTION ) override;

        virtual void reset() override;

        virtual void sleep( int mode ) override;

    private:
        regBits_t m_PS;
        regBits_t m_PSA;

};
#endif
