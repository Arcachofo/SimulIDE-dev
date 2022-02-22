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

#ifndef SCR_H
#define SCR_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class eDiode;
class eResistor;

class MAINMODULE_EXPORT SCR : public Component, public eElement
{
        Q_OBJECT
    public:
        SCR( QObject* parent, QString type, QString id );
        ~SCR();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        double gateRes() { return m_gateRes; }
        void setGateRes( double r ) { m_gateRes = r; }

        double trigCurr() { return m_trigCurr; }
        void setTrigCurr( double c ) { m_trigCurr = c; }

        double holdCurr() { return m_holdCurr; }
        void setHoldCurr( double v ) { m_holdCurr = v; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        double m_gateRes;
        double m_trigCurr;
        double m_holdCurr;

        bool m_state;

        eNode*     m_midEnode;
        eResistor* m_resistorA;
        eResistor* m_resistorC;
        eDiode*    m_diode;
};

#endif
