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

#ifndef LOGICINPUT_H
#define LOGICINPUT_H

#include "component.h"
#include "e-element.h"

class IoPin;
class LibraryItem;

class MAINMODULE_EXPORT LogicInput : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double Voltage READ volt WRITE setVolt DESIGNABLE true USER true )
    Q_PROPERTY( bool   Out     READ out  WRITE setOut  DESIGNABLE true)

    public:
        LogicInput( QObject* parent, QString type, QString id );
        ~LogicInput();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        bool out() { return m_button->isChecked(); }
        virtual void setOut( bool out );

        double volt() { return m_value; }
        void setVolt( double v );
        
        virtual void setUnit( QString un ) override;

        virtual void stamp() override;
        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        virtual void onbuttonclicked();

    protected:
        void updateOutput();

        IoPin* m_outpin;

        QToolButton* m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif
