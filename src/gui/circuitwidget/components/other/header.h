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

#ifndef HEADER_H
#define HEADER_H

#include "connbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Header : public ConnBase
{
    Q_OBJECT
    public:
        Header( QObject* parent, QString type, QString id );
        ~Header();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        bool pointDown() { return m_pointDown; }
        void setPointDown( bool down );

    private:
        virtual void updatePixmap() override;

        bool m_pointDown;
};

#endif
