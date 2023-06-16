/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef KS0108_H
#define KS0108_H

#include "component.h"
#include "e-element.h"
#include "iopin.h"

class LibraryItem;

class MAINMODULE_EXPORT Ks0108 : public Component, public eElement
{
    public:
        Ks0108( QObject* parent, QString type, QString id );
        ~Ks0108();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        void setCsActLow( bool low ) { m_csActLow = low; }
        bool csActLow() { return m_csActLow; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void initPins();
        void writeData( int data );
        void proccessCommand( int command );
        void ReadData();
        void ReadStatus();
        void dispOn( int state );
        void setYaddr( int addr );
        void setXaddr( int addr );
        void startLin( int line ) { m_startLin = line; }
        void clearLcd();
        void clearDDRAM();
        void incrementPointer();
        void reset();

        QImage *m_pdisplayImg;        //Visual representation of the LCD

        unsigned char m_aDispRam[8][128];                 //128x64 DDRAM
        
        int m_input;
        int m_addrX1;                                   // X RAM address
        int m_addrY1;                                   // Y RAM address
        int m_addrX2;                                   // X RAM address
        int m_addrY2;                                   // Y RAM address
        int m_startLin;
        
        bool m_csActLow;
        bool m_Cs1;
        bool m_Cs2;
        bool m_dispOn;
        bool m_lastScl;
        bool m_reset;
        bool m_Write;

        //Inputs
        Pin m_pinRst;
        Pin m_pinCs2;
        Pin m_pinCs1;
        Pin m_pinEn;
        Pin m_pinRW;
        Pin m_pinDC;
        
        std::vector<IoPin*> m_dataPin;
};

#endif
