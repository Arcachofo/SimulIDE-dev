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

#ifndef EELEMENT_H
#define EELEMENT_H

#include <vector>
#include <string>
#include <math.h>
#include "e-pin.h"


class MAINMODULE_EXPORT eElement
{
    public:
        eElement( QString id );
        virtual ~eElement();

        virtual void initialize(){;}
        virtual void attach(){;}
        virtual void stamp(){;}

        virtual void runEvent(){;}
        virtual void updateStep(){;}
        virtual void voltChanged(){;}

        virtual void setNumEpins( int n );

        virtual ePin* getEpin( int pin );
        virtual void setEpin( int num, ePin* pin );

        QString getId(){ return m_elmId; }

        bool converged() { return m_converged; }
        bool m_converged;

        static constexpr double cero_doub = 1e-14;
        static constexpr double high_imp  = 1e14;

        double digital_high;
        double digital_low;
        double digital_thre;

        // Simulator engine
        eElement* nextChanged;
        eElement* nextNonLin;
        bool added;

    protected:
        std::vector<ePin*> m_ePin;

        QString m_elmId;

        bool m_changed;
};

#endif

