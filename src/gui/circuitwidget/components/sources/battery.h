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

#ifndef BATTERY_H
#define BATTERY_H

#include "e-battery.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT Battery : public Component, public eBattery
{
    Q_OBJECT
    Q_PROPERTY( double Voltage READ volt WRITE setVolt DESIGNABLE true USER true )

    public:
        Battery( QObject* parent, QString type, QString id );
        ~Battery();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual double volt() override;
        virtual void setVolt( double volt ) override;
        virtual void setUnit( QString un ) override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
};

#endif
