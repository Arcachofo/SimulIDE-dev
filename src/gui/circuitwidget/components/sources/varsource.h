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

#ifndef VARSOURCE_H
#define VARSOURCE_H

#include "component.h"
#include "voltwidget.h"
#include "e-element.h"

class MAINMODULE_EXPORT VarSource : public Component, public eElement
{
    Q_OBJECT
    public:
        VarSource( QObject* parent, QString type, QString id );
        ~VarSource();

        double getVal() { return m_outValue; }
        void setVal( double val );

        double maxValue() { return m_maxValue; }
        void setMaxValue( double v ) { m_maxValue = v;}

        virtual void initialize() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void valueChanged( int val );

    protected:
        void updateButton();
        void updtValue( double v );
        
        double m_maxValue;
        double m_outValue;

        QString m_unit;
        
        VoltWidget m_voltw;

        QPushButton* m_button;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif

