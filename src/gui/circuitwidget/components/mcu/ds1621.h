/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#ifndef DS1621_H
#define DS1621_H

#include "twimodule.h"
#include "iocomponent.h"

class LibraryItem;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT DS1621 : public IoComponent, public TwiModule
{
    Q_OBJECT
    
    public:
        DS1621( QObject* parent, QString type, QString id );
        ~DS1621();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double temp() { return m_temp; }
        void setTemp( double temp ) { m_temp = temp; }

        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void readByte() override;
        virtual void writeByte() override;
        //virtual void startWrite() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        double trim( double data ) { return (double)((int)(data*10))/10; }
        void doConvert();

        bool m_convert;
        bool m_oneShot;
        bool m_outPol;

        int m_writeByte;
        uint8_t m_command;

        uint8_t m_config;
        uint8_t m_tempCount;
        uint8_t m_tempSlope;

        double m_tempInc;
        double m_temp;
        int8_t m_tempReg[2];

        double m_Th;
        int8_t m_ThReg[2];

        double m_Tl;
        int8_t m_TlReg[2];

        QFont m_font;
};

#endif

