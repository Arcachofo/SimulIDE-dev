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

#ifndef LAMP_H
#define LAMP_H

#include "e-resistor.h"
#include "comp2pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Lamp : public Comp2Pin, public eResistor
{
        Q_OBJECT
    public:
        Lamp( QObject* parent, QString type, QString id );
        ~Lamp();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        double maxCurrent()             { return m_maxCurrent; }
        void  setMaxCurrent( double c ) { m_maxCurrent = c; }

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        void updateVI();
        void updateBright();
        double overCurrent() { return m_avgCurrent/m_maxCurrent; }

        uint64_t m_prevStep;
        uint32_t m_intensity;

        double m_maxCurrent;
        double m_avgCurrent;
        double m_totalCurrent;
        double m_lastCurrent;
        double m_lastPeriod;
        double m_brightness;
};

#endif

