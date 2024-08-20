/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SSD1306_H
#define SSD1306_H

#include "twimodule.h"
#include "component.h"

#define HORI_ADDR_MODE 0
#define VERT_ADDR_MODE 1
#define PAGE_ADDR_MODE 2

class LibraryItem;
class IoPin;

class Ssd1306 : public Component, public TwiModule
{
    public:
        Ssd1306( QString type, QString id );
        ~Ssd1306();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        enum dispColor {
            White = 0,
            Blue,
            Yellow
        };
        QString colorStr() { return m_enumUids.at((int)m_dColor ); }
        void setColorStr( QString color );

        int width() { return m_width; }
        void setWidth( int w );

        int height() { return m_height; }
        void setHeight( int h );

        bool imgRotated() { return m_rotate; }
        void setImgRotated( bool r ) { m_rotate = r; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        virtual void startWrite() override;
        virtual void readByte() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        void writeData();
        void proccessCommand();
        void incrementPointer();
        void reset();
        void clearDDRAM();
        void updateSize();
        void scroll();

        dispColor m_dColor;

        uint8_t m_aDispRam[128][16]; //128x128 DDRAM

        int m_cdr;       // Clock Divide Ratio
        int m_mr;        // Multiplex Ratio
        int m_fosc;      // Oscillator Frequency
        int m_frm;       // Frame Frequency

        int m_width;
        int m_height;
        int m_rows;

        int m_addrX;     // X RAM address
        int m_addrY;     // Y RAM address
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
        bool m_scanInv;
        bool m_rotate;

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

        QColor m_foreground;
};

#endif
