/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICWDT_H
#define PICWDT_H

#include "mcuwdt.h"

class MAINMODULE_EXPORT PicWdt : public McuWdt
{
    public:
        PicWdt( eMcu* mcu, QString name );
        ~PicWdt();

        virtual void initialize() override;
        //virtual void runEvent() override;

        virtual void configureA( uint8_t newOPTION ) override;

        virtual void reset() override;

    private:
        regBits_t m_PS;
        regBits_t m_PSA;

};
#endif
