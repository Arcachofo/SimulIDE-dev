/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "component.h"
#include "e-resistor.h"

class LibraryItem;

class MAINMODULE_EXPORT DcMotor : public Component, public eResistor
{
    public:
        DcMotor( QObject* parent, QString type, QString id );
        ~DcMotor();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

        int rpm() { return m_rpm; }
        void setRpm( int rpm );

        double volt() { return m_voltNom; }
        void setVolt( double v ) { m_voltNom = v; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        void updatePos();

        int m_rpm;

        double m_LastVolt;
        double m_voltNom;
        double m_ang;
        double m_motStPs;
        double m_speed;
        double m_delta;

        uint64_t m_lastTime;
        uint64_t m_updtTime;
};

#endif
