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

#include "itemlibrary.h"
#include "component.h"
#include "e-resistor.h"


class MAINMODULE_EXPORT DcMotor : public Component, public eResistor
{
    Q_OBJECT
    Q_PROPERTY( int     RPM_Nominal  READ rpm    WRITE setRpm    DESIGNABLE true USER true )
    Q_PROPERTY( double  Volt_Nominal READ volt   WRITE setVolt   DESIGNABLE true USER true )
    Q_PROPERTY( double  Resistance   READ resist WRITE setResist DESIGNABLE true USER true )

    public:
        DcMotor( QObject* parent, QString type, QString id );
        ~DcMotor();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        int rpm();
        void setRpm(int rpm );

        double volt();
        void setVolt( double volt );
        
        double resist();
        void setResist( double res );
        
        void setUnit( QString un );

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;
        
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

