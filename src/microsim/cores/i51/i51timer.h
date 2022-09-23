/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef I51TIMER_H
#define I51TIMER_H

#include "mcutimer.h"

class MAINMODULE_EXPORT I51Timer : public McuTimer
{
    public:
        I51Timer( eMcu* mcu, QString name );
        ~I51Timer();

        virtual void initialize() override;
        virtual void configureA( uint8_t newTMOD ) override;

        virtual void updtCycles() override;
        virtual void updtCount( uint8_t val=0 ) override;

    protected:
        bool m_gate;

        regBits_t m_TxM;
        regBits_t m_CTx;
        regBits_t m_GATE;
};

#endif
