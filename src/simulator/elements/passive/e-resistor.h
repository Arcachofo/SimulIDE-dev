/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef ERESISTOR_H
#define ERESISTOR_H

#include "e-element.h"

class MAINMODULE_EXPORT eResistor : public eElement
{
    public:
        eResistor( QString id );
        ~eResistor();

        virtual void stamp() override;

        virtual double res() { return 1/m_admit; }
        virtual void setRes( double resist );
        double getRes() { return 1/m_admit; }
        virtual void setResSafe( double resist );
        
        double admit() { return m_admit; }
        void setAdmit( double admit );
        void stampAdmit();
        //void stampCurrent( double current );

        double current();

    protected:
        virtual void updateVI();

        double m_admit;
        double m_current;
};

#endif
