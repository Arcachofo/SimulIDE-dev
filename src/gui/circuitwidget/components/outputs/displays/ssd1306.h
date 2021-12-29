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

#ifndef SSD1306_H
#define SSD1306_H

#include "twimodule.h"
#include "component.h"

#define HORI_ADDR_MODE 0
#define VERT_ADDR_MODE 1
#define PAGE_ADDR_MODE 2

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Ssd1306 : public Component, public TwiModule
{
        Q_OBJECT
    public:
        Ssd1306( QObject* parent, QString type, QString id );
        ~Ssd1306();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        enum dispColor {
            White = 0,
            Blue,
            Yellow
        };
        void setColor( QString c );
        QString color()  { return m_colors.at( (int)m_dColor ); }

        int cCode() { return m_address; }
        void setCcode( int code ) { m_address = code; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void remove() override;

        virtual void readByte() override;
        virtual void I2Cstop() override;

        virtual QStringList getEnums( QString e ) override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        void writeData();
        void proccessCommand();
        void clearLcd();
        void incrementPointer();
        void reset();
        void clearDDRAM();
        
        QImage* m_pdisplayImg;    //Visual representation of the LCD
        dispColor m_dColor;

        unsigned char m_aDispRam[128][8]; //128x64 DDRAM

        int m_cdr;                // Clock Divide Ratio
        int m_mr;                 // Multiplex Ratio
        int m_fosc;               // Oscillator Frequency
        int m_frm;                // Frame Frequency

        int m_addrX;              // X RAM address
        int m_addrY;              // Y RAM address
        int m_startX;
        int m_endX;
        int m_startY;
        int m_endY;

        int m_startLin;
        int m_addrMode;
        int m_lastCommand;

        int m_scrollStartPage;
        int m_scrollEndPage;
        int m_scrollInterval;
        int m_scrollVertOffset;
        int m_scrollCount;

        int m_readBytes;

        bool m_dispOn;
        bool m_dispFull;
        bool m_dispInv;

        //bool m_reset;
        bool m_command;
        bool m_data;
        bool m_continue;
        bool m_scroll;
        bool m_scrollR;
        bool m_scrollV;

        //Inputs
        IoPin* m_pinSda;
        //Pin m_pinRst;
        //Pin m_pinDC;
        //Pin m_pinCS;

 static QStringList m_colors;
};

#endif
