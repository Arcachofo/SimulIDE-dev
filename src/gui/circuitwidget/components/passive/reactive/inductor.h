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

#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "reactive.h"

class LibraryItem;

class MAINMODULE_EXPORT Inductor : public Reactive
{
        Q_OBJECT
    public:
        Inductor( QObject* parent, QString type, QString id );
        ~Inductor();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        double indCurrent() { return m_curSource; }

        Pin* getPin( int n ) { return m_pin[n]; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        virtual double updtRes()  override { return m_value/m_tStep; }
        virtual double updtCurr() override { return m_curSource - m_volt*m_admit; }
};

#endif
