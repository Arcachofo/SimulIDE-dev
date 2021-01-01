/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#ifndef SR04_H
#define SR04_H

#include "e-source.h"
#include "e-element.h"
#include "component.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT SR04 : public Component, public eElement
{
    Q_OBJECT
    
    public:

        SR04( QObject* parent, QString type, QString id );
        ~SR04();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        virtual void remove() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
        uint64_t m_lastStep;
        bool     m_lastTrig;

        int m_echouS;
        
        Pin* m_inpin;
        Pin* m_trigpin;
        Pin* m_echopin;
        
        eSource* m_echo;
};

#endif

