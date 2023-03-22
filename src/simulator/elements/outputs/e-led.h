/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ELED_H
#define ELED_H

#include "e-resistor.h"

class MAINMODULE_EXPORT eLed : public eResistor
{
    public:
        eLed( QString id );
        ~eLed();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

        double threshold() { return m_threshold; }
        void  setThreshold( double threshold ) { m_threshold = threshold; m_changed = true; }

        double maxCurrent()             { return m_maxCurrent; }
        void  setMaxCurrent( double c ) { m_maxCurrent = c; m_changed = true; }

        virtual void   setRes( double resist );
        virtual double res() { return m_imped; }

        double brightness() { return m_brightness; }
        double overCurrent() { return m_avgCurrent/m_maxCurrent; }

        void updateBright();

    protected:
        virtual void updateVI() override;

        uint64_t m_prevStep;
        uint32_t m_intensity;

        double m_maxCurrent;
        double m_avgCurrent;
        double m_totalCurrent;
        double m_lastCurrent;
        double m_lastPeriod;
        double m_brightness;

        double m_voltPN;
        double m_threshold;
        double m_imped;
        double m_lastThCurrent;
};

#endif
