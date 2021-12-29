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

#include "e-element.h"
#include "comp2pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Battery : public Comp2Pin, public eElement
{
        Q_OBJECT
    public:
        Battery( QObject* parent, QString type, QString id );
        ~Battery();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        double volt() { return m_volt; }
        void setVolt( double volt );

        virtual void stamp() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
        double m_volt;
};

#endif
