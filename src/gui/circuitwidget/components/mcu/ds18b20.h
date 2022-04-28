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

#ifndef DS18B20_H
#define DS18B20_H

#include <QFont>

#include "e-element.h"
#include "component.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Ds18b20 : public Component , public eElement
{
    Q_OBJECT

    public:
        Ds18b20( QObject* parent, QString type, QString id );
        ~Ds18b20();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double temp() { return m_temp; }
        void setTemp( double temp ) { m_temp = temp; }

        virtual void stamp();
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        void byteReceived();
        void byteSent();
        void sendByte( uint8_t data );
        void writeBit();
        void readBit( uint8_t bit );
        void reset();

        double trim( double data ) { return (double)((int)(data*10))/10; }

        bool m_lastIn;
        bool m_write;
        bool m_reset;
        bool m_pullDown;

        uint64_t m_lastTime;
        uint8_t  m_data;
        uint8_t  m_bit;

        int m_pulse;

        double m_temp;
        double m_tempInc;

        QFont m_font;

        IoPin* m_inpin;
};

#endif
