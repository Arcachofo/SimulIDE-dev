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

#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include "e-element.h"
#include "component.h"
#include "e-resistor.h"
#include "touchpadwidget.h"
#include "pin.h"

class LibraryItem;
class Pin;

class MAINMODULE_EXPORT TouchPad : public Component, public eElement
{
    Q_OBJECT

    public:
        TouchPad( QObject* parent, QString type, QString id );
        ~TouchPad();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        int width() { return m_width; }
        void setWidth( int w );

        int height() { return m_height; }
        void setHeight( int h );

        bool transparent() { return m_transparent; }
        void setTransparent( bool t );

        double RxMin() { return m_RxMin; }
        void setRxMin( double min );

        double RxMax() { return m_RxMax; }
        void setRxMax( double max );

        double RyMin() { return m_RyMin; }
        void setRyMin( double min );

        double RyMax() { return m_RyMax; }
        void setRyMax( double max );

        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        void updateSize();

        bool m_transparent;

        int m_width;
        int m_height;
        int m_xPos;
        int m_yPos;

        double m_RxMin;
        double m_RxMax;
        double m_RyMin;
        double m_RyMax;

        eNode* m_midEnode;

        eResistor m_resXA;
        eResistor m_resXB;
        eResistor m_resYA;
        eResistor m_resYB;

        Pin* m_vrx_p;
        Pin* m_vry_p;
        Pin* m_vrx_m;
        Pin* m_vry_m;

        ePin m_ePinXA;
        ePin m_ePinXB;
        ePin m_ePinYA;
        ePin m_ePinYB;

        TouchPadWidget m_touchpadW;
        QGraphicsProxyWidget* m_proxy;
};

#endif
