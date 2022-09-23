/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ILI9341_H
#define ILI9341_H

#include "component.h"
#include "e-clocked_device.h"
#include "iopin.h"

class LibraryItem;

class MAINMODULE_EXPORT Ili9341 : public Component, public eClockedDevice
{
    public:
        Ili9341( QObject* parent, QString type, QString id );
        ~Ili9341();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;
        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        void proccessCommand();
        void getParameter();
        void clearLcd();
        void incrementPointer();
        void incrementY();
        void reset();
        void clearDDRAM();
        
        QImage* m_pdisplayImg;    //Visual representation of the LCD

        unsigned char m_rxReg;     // Received value
        unsigned int m_aDispRam[240][320]; // DDRAM

        int m_inBit;        //How many bits have we read since last byte
        int m_inByte;
        uint m_data;
        int m_dataBytes;

        int m_addrX;              // X RAM address
        int m_addrY;              // Y RAM address
        int m_startX;
        int m_endX;
        int m_startY;
        int m_endY;
        int m_maxX;
        int m_maxY;
        int m_dirX;
        int m_dirY;

        int m_startLin;
        //int m_addrMode;
        int m_lastCommand;

        int m_scrollStartPage;
        int m_scrollEndPage;
        int m_scrollInterval;
        int m_scrollVertOffset;
        int m_scrollCount;

        int m_readBytes;

        bool m_dispOn;
        //bool m_dispFull;
        bool m_dispInv;

        //bool m_reset;
        bool m_scroll;
        bool m_scrollR;
        bool m_scrollV;
        bool m_RGB;

        //Inputs
        IoPin m_pinCS;
        IoPin m_pinRst;
        IoPin m_pinDC;
        IoPin m_pinMosi;
        IoPin m_pinSck;
        //Pin m_pinMiso;
};

#endif
