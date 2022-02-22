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

#ifndef DIAC_H
#define DIAC_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class eDiode;
class eResistor;

class MAINMODULE_EXPORT Diac : public Component, public eElement
{
        Q_OBJECT
    public:
        Diac( QObject* parent, QString type, QString id );
        ~Diac();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        double resOn() { return m_resOn; }
        void setResOn( double r ) { m_resOn = r; }

        double resOff() { return m_resOff; }
        void setResOff( double r ) { m_resOff = r; }

        double brkVolt() { return m_brkVolt; }
        void setBrkVolt( double v ) { m_brkVolt = v; }

        double holdCurr() { return m_holdCurr; }
        void setHoldCurr( double v ) { m_holdCurr = v; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:

        double m_resOn;
        double m_resOff;
        double m_brkVolt;
        double m_holdCurr;

        bool m_state;

        eNode* m_midEnode;
        eResistor* m_resistor;

        eDiode* m_diode1;
        eDiode* m_diode2;
};

#endif

