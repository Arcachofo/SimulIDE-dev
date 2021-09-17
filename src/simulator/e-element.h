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
#include <QString>

class ePin;

class MAINMODULE_EXPORT eElement
{
    public:
        eElement( QString id );
        virtual ~eElement();

        virtual void initialize(){;}
        virtual void attach(){;}
        virtual void stamp(){;}

        virtual void runEvent(){;}
        virtual void voltChanged(){;}

        virtual void setNumEpins( int n );

        virtual ePin* getEpin( int num );
        virtual void setEpin( int num, ePin* pin );

        QString getId(){ return m_elmId; }

        static constexpr double cero_doub = 1e-14;
        static constexpr double high_imp  = 1e14;

        // Simulator engine
        eElement* nextChanged;
        bool added;

    protected:
        std::vector<ePin*> m_ePin;

        QString m_elmId;

        bool m_changed;
        double m_step;
};

#endif

