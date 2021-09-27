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

#ifndef FIXEDVOLT_H
#define FIXEDVOLT_H

#include <QToolButton>
#include <QGraphicsProxyWidget>

#include "component.h"
#include "e-element.h"

class IoPin;
class LibraryItem;
class QToolButton;

class MAINMODULE_EXPORT FixedVolt : public Component, public eElement
{
        Q_OBJECT
    public:
        FixedVolt( QObject* parent, QString type, QString id );
        ~FixedVolt();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;

        bool out();
        virtual void setOut( bool out );

        double volt() { return m_voltage; }
        void setVolt( double v );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        virtual void onbuttonclicked();

    protected:
        void updateOutput();

        double m_voltage;

        IoPin* m_outpin;

        QToolButton* m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif
