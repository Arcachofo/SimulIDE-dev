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

#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include "component.h"
#include "e-resistor.h"
#include "e-element.h"
#include "dialwidget.h"
#include "pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Potentiometer : public Component, public eElement
{
        Q_OBJECT
    public:
        Potentiometer( QObject* parent, QString type, QString id );
        ~Potentiometer();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double getVal();
        void setVal( double val );

        double getRes() { return m_resist; }
        void setRes( double v );

        virtual void stamp() override;
        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void resChanged( int volt );

    private:
        double m_resist;
        double m_voltOut;
        double m_res1;

        bool m_changed;
        
        Pin m_pinA;
        Pin m_pinM;
        Pin m_pinB;
        ePin m_ePinA;
        ePin m_ePinB;
        
        eResistor m_resA;
        eResistor m_resB;
        
        eNode* m_midEnode;
        
        QDial* m_dial;
        DialWidget m_dialW;
        QGraphicsProxyWidget* m_proxy;
};

#endif
