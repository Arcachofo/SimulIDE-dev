/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2016 by santiago González                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OPAMP_H
#define OPAMP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT OpAmp : public Component, public eElement
{
        Q_OBJECT
    public:
        OpAmp( QObject* parent, QString type, QString id );
        ~OpAmp();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        double gain() { return m_gain; }
        void setGain( double g ) { m_gain = g; }

        double outImp() { return m_outImp; }
        void setOutImp( double imp );

        double voltPos() { return m_voltPosDef; }
        void setVoltPos( double v ) { m_voltPosDef = v; }

        double voltNeg() { return m_voltNegDef; }
        void setVoltNeg( double v ) { m_voltNegDef = v; }

        bool powerPins() {return m_powerPins; }
        void setPowerPins( bool set );

        bool switchPins() { return m_switchPins; }
        void setSwitchPins( bool s );

        virtual QPainterPath shape() const;
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        bool m_powerPins;
        bool m_switchPins;

        double m_accuracy;
        double m_gain;
        double m_k;
        double m_voltPos;
        double m_voltNeg;
        double m_voltPosDef;
        double m_voltNegDef;
        double m_lastOut;
        double m_lastIn;
        double m_outImp;

        IoPin* m_inputP;
        IoPin* m_inputN;
        IoPin* m_output;
};

#endif
