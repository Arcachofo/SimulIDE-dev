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

#ifndef KY040_H
#define KY040_H

#include "e-element.h"
#include "component.h"
#include "dialwidget.h"

class LibraryItem;
class QToolButton;
class IoPin;

class MAINMODULE_EXPORT KY040 : public Component, public eElement
{
    Q_OBJECT
    
    public:
        KY040( QObject* parent, QString type, QString id );
        ~KY040();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void runEvent() override;

        int steps() { return m_steps; }
        void setSteps( int s );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    public slots:
        void onbuttonchanged();

    private:
        int m_steps;
        int m_prevDialVal;
        int m_delta;
        uint64_t m_stepDelta;
        int m_posA;
        int m_posB;

        bool m_stateA;
        bool m_stateB;
        
        bool m_changed;
        
        DialWidget m_dialW;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
        
        QToolButton* m_button;
        QGraphicsProxyWidget* m_proxy_button;

        IoPin* m_pinA;
        IoPin* m_pinB;
        IoPin* m_sw;
};

#endif
