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

#ifndef EBATTERY_H
#define EBATTERY_H

#include "e-element.h"

class MAINMODULE_EXPORT eBattery : public eElement
{
    public:
        eBattery( QString id );
        ~eBattery();

        virtual double volt();
        virtual void setVolt( double volt );

        virtual void stamp() override;
        virtual void initialize() override;

    protected:

        double m_volt;
        double m_accuracy;
        double m_lastOut;
};

#endif
