/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef OSCOPECHANNEL_H
#define OSCOPECHANNEL_H

#include "datachannel.h"
#include "oscope.h"

class MAINMODULE_EXPORT OscopeChannel : public DataChannel
{
        friend class Oscope;

    public:

        OscopeChannel( Oscope* oscope, QString id );
        ~OscopeChannel();

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void setFilter( double f ) override;

    private:
        void updateValues();

        uint64_t m_totalP;
        uint64_t m_lastMax;
        uint64_t m_numMax;       // Number of Maximum found
        uint64_t m_nCycles;
        uint64_t m_simTime;

        double m_lastValue;
        double m_freq;
        double m_mid;

        double m_maxVal;
        double m_minVal;
        double m_midVal;
        double m_dispMax;
        double m_dispMin;
        double m_ampli;
        double m_filter;

        Oscope* m_oscope;
};

#endif

