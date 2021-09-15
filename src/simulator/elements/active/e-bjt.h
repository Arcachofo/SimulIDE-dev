/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef EBJT_H
#define EBJT_H

#include "e-element.h"

class MAINMODULE_EXPORT eBJT : public eElement
{
    public:

        eBJT( QString id );
        virtual ~eBJT();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        virtual double gain()              { return m_gain; }
        virtual void setGain( double gain ){ m_gain = gain; }

        virtual double pnp()              { return m_PNP; }
        virtual void setPnp( double pnp ) { m_PNP = pnp; }

    protected:
        double limitStep( double vnew, double vold );

        //double m_accuracy;
        double m_baseCurr;
        double m_voltE;
        double m_voltBE;
        double m_voltBC;
        double m_gain;
        double m_vt;
        double m_rsCurr;
        double m_thr;
        double m_rgain;
        double m_fgain;
        bool m_PNP;

        eElement* m_BEjunction;
        eElement* m_BCjunction;

        ePin* m_BC;
        ePin* m_CB;
        ePin* m_BE;
        ePin* m_EB;
        ePin* m_CE;
        ePin* m_EC;

        double gBC;
        double gCB;
        double gBE;
        double gEB;

        double curB;
        double curC;
        double curE;
};

#endif
