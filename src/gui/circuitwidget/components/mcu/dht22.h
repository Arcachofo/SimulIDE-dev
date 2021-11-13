/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef DHT22_H
#define DHT22_H

//#include "e-logic_device.h"
#include "e-element.h"
#include "component.h"

class LibraryItem;
class QPushButton;
class QGraphicsProxyWidget;
class IoPin;

class MAINMODULE_EXPORT Dht22 : public Component , public eElement
{
    Q_OBJECT

    public:
        Dht22( QObject* parent, QString type, QString id );
        ~Dht22();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        void setModel( QString model);
        QString model();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double humidInc() { return m_humiInc; }
        void setHumidInc( double inc ) { m_humiInc = trim( inc ); }

        virtual void stamp();
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual QStringList getEnums( QString e ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        void calcData();
        double trim( double data );

        bool m_DHT22;
        bool m_lastIn;
        bool m_set;

        uint64_t m_lastTime;
        uint64_t m_start;
        uint64_t m_data;
        uint64_t m_bit;

        double m_temp;
        double m_humi;
        double m_tempInc;
        double m_humiInc;

        int m_outStep;
        int m_bitStep;

        QFont m_font;

        QPushButton* m_button;
        QGraphicsProxyWidget* m_proxy;

        IoPin* m_inpin;
};

#endif
