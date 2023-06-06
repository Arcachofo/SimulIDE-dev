/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRCOMPARATOR_H
#define AVRCOMPARATOR_H

#include "mcucomparator.h"
#include "mcutypes.h"

class MAINMODULE_EXPORT AvrComp : public McuComp
{
    public:
        AvrComp( eMcu* mcu, QString name );
        ~AvrComp();

        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void configureA( uint8_t newACSR ) override;
        virtual void configureB( uint8_t newAIND ) override;
        virtual void configureC( uint8_t newACOE ) override;

        virtual void setPinN( McuPin* pin ) override;

        void readACO( uint8_t );

    protected:
        void compare( uint8_t v=0 );
        void changeCallbacks();

        bool m_acie;
        bool m_acoe;

        regBits_t m_ACD;
        regBits_t m_ACBG;
        regBits_t m_ACO;
        regBits_t m_ACI;
        regBits_t m_ACIE;
        regBits_t m_ACIC;
        regBits_t m_ACIS;

        regBits_t m_AIN0D;
        regBits_t m_AIN1D;
};

#endif
