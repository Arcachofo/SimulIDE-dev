/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef EMOSFET_H
#define EMOSFET_H

#include "e-resistor.h"

class MAINMODULE_EXPORT eMosfet : public eResistor
{
    public:
        eMosfet( QString id );
        ~eMosfet();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

        bool pChannel() { return m_Pchannel; }
        void setPchannel( bool p ) { m_Pchannel = p; }

        bool depletion() { return m_depletion; }
        void setDepletion( bool d ) { m_depletion = d; }

        double rdson() { return m_RDSon; }
        void setRDSon( double rdson );

        double threshold() { return m_threshold; }
        void setThreshold( double th );
        
    protected:
        double m_accuracy;
        double m_lastCurrent;
        double m_threshold;
        double m_kRDSon;
        double m_RDSon;
        double m_gateV;
        double m_Gth;

        bool m_Pchannel;
        bool m_depletion;
};

#endif
