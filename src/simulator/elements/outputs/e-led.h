/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
        void  setThreshold( double threshold ) { m_threshold = threshold; }

        double maxCurrent()             { return m_maxCurrent; }
        void  setMaxCurrent( double c ) { m_maxCurrent = c; }

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
