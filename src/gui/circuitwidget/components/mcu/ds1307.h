/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef DS1307_H
#define DS1307_H

#include "twimodule.h"
#include "component.h"
#include "rtclock.h"

class LibraryItem;

class MAINMODULE_EXPORT DS1307 : public Component, public TwiModule
{
    public:
        DS1307( QObject* parent, QString type, QString id );
        ~DS1307();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;

        virtual void writeByte() override;
        virtual void readByte() override;
        virtual void I2Cstop() override;

        char decToBcd(char val);
        char bcdToDec(char val);

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        void updtTime();
        void updtDate();
        void updtCtrl();

        uint8_t m_addrPtr;
        uint8_t m_data[64];

        int m_phase = 0;

        IoPin* m_pinSda;
        IoPin* m_outpin;

        RtClock m_clock;
};

#endif
