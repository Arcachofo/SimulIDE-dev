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

#ifndef CONNBASE_H
#define CONNBASE_H

#include "component.h"
#include "e-element.h"
#include "pin.h"


class MAINMODULE_EXPORT ConnBase : public Component, public eElement
{
    Q_OBJECT
    public:
        ConnBase( QObject* parent, QString type, QString id );
        ~ConnBase();


        int  size() { return m_size; }
        void setSize( int size );

        //virtual void remove() override;

        virtual void registerEnode( eNode*, int n=-1 ) override;

        virtual void setHidden( bool hid, bool hidLabel=false ) override;

        void createPins( int c );
        void deletePins( int d );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        virtual void updatePixmap(){;}

        int m_size;

        std::vector<Pin*> m_connPins;
        std::vector<Pin*> m_sockPins;

        Pin::pinType_t m_pinType;

        QPixmap m_pinPixmap;
};

#endif
