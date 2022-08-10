/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#ifndef REACTIVE_H
#define REACTIVE_H

#include "e-reactive.h"
#include "comp2pin.h"

class MAINMODULE_EXPORT Reactive : public Comp2Pin, public eReactive
{
        Q_OBJECT
    public:
        Reactive( QObject* parent, QString type, QString id );
        ~Reactive();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        virtual void stepError() override;

        double value() { return m_value; }
        void setValue( double c );

        double resist() { return m_resistor->res(); }
        void setResist( double resist );

        int autoStep() { return m_autoStep; }
        void setAutoStep( int a );

    private:
        eNode* m_midEnode;
        eResistor* m_resistor;
};

#endif
