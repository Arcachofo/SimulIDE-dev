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

#include <string>
#include <math.h>
#include <QPointer>
#include <QDebug>
#include "e-pin.h"

#define Trig_None 0
#define Trig_Clk  1
#define Trig_InEn 2

#define Clock_Low      0
#define Clock_Rising   1
#define Clock_Allow    1
#define Clock_High     2
#define Clock_Falling  3

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

        virtual uint64_t propDelay() { return m_propDelay/1000; }
        virtual void     setPropDelay( uint64_t pd ) { m_propDelay = pd*1000; }

        virtual void setNumEpins( int n );

        virtual ePin* getEpin( int pin );
        virtual void setEpin( int num, ePin* pin );

        QString getId(){ return m_elmId; }

        bool converged() { return m_converged; }

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
        bool m_converged;

        uint64_t m_propDelay; // Propagation delay
};

#endif

