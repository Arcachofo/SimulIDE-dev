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
#include "e-source.h"
#include "voltwidget.h"
#include "pin.h"


class MAINMODULE_EXPORT VarSource : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( QString Unit      READ unit    WRITE setUnit    DESIGNABLE true USER true )
    Q_PROPERTY( bool    Show_Volt READ showVal WRITE setShowVal DESIGNABLE true USER true )

    public:
        VarSource( QObject* parent, QString type, QString id );
        ~VarSource();

        double value() const      { return m_value; }
        void setValue( double v );
        
        void setUnit( QString un );

        virtual void initialize() override;
        virtual void remove() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void valueChanged( int val );

    protected:
        void updateButton();
        void updtValue( double v );
        
        double m_maxValue;
        double m_outValue;

        Pin*     m_outpin;
        eSource* m_out;
        
        VoltWidget m_voltw;

        QPushButton* m_button;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif

