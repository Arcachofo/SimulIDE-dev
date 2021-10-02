/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                      *
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
/*
 *   Modified 2020 by santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#ifndef KY023_H
#define KY023_H

#include "e-element.h"
#include "component.h"
#include "joystickwidget.h"

class LibraryItem;
class IoPin;
class QToolButton;

class MAINMODULE_EXPORT KY023 : public Component, public eElement
{
    Q_OBJECT
    
    public:

        KY023( QObject* parent, QString type, QString id );
        ~KY023();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void updateStep() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    public slots:
        void onbuttonpressed();
        void onbuttonreleased();
        void onvaluechanged(int xValue, int yValue);
        
    private:
        JoystickWidget m_joystickW;
        QGraphicsProxyWidget* m_proxy;
        
        QToolButton* m_button;
        QGraphicsProxyWidget* m_proxy_button;
        
        bool m_changed;
        bool m_closed;
        
        double m_vOutX;
        double m_vOutY;
        
        IoPin* m_vrx;
        IoPin* m_vry;
        IoPin* m_sw;
};

#endif

