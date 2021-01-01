/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef OSCOPE_H
#define OSCOPE_H

#include "plotbase.h"


class LibraryItem;

class MAINMODULE_EXPORT Oscope : public PlotBase
{
    Q_OBJECT
    Q_PROPERTY( double Filter   READ filter  WRITE setFilter DESIGNABLE true USER true )

    public:

        Oscope( QObject* parent, QString type, QString id );
        ~Oscope();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double filter() { return m_filter; }
        void setFilter( double filter );

        //virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:

        double m_filter;
};

#endif

