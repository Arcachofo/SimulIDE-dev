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

#ifndef RAIL_H
#define RAIL_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Rail : public Component, public eElement
{
    public:
        Rail( QObject* parent, QString type, QString id );
        ~Rail();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double volt() { return m_volt; }
        void setVolt( double v );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void updateOutput();
        double m_volt;

        IoPin* m_out;
};

#endif
