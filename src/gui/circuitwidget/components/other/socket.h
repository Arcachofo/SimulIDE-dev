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

#ifndef SOCKET_H
#define SOCKET_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class Pin;

class MAINMODULE_EXPORT Socket : public Component, public eElement
{
    Q_OBJECT
    public:
        Socket( QObject* parent, QString type, QString id );
        ~Socket();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        int  size() { return m_size; }
        void setSize( int size );

        virtual void stamp() override;
        virtual void updateStep() override;

        void createSwitches( int c );
        void deleteSwitches( int d );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        void updatePins( bool connect );
        int m_size;

        std::vector<Pin*> m_connPins;
};

#endif
