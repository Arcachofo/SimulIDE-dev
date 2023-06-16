/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
// Based on:

// C++ Interface: pcd8544
//
// Description: This component emulates a graphic LCD module based on the
//              PCD8544 controller.
//
// Author: Roland Elek <elek.roland@gmail.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution


#ifndef PCD8544_H
#define PCD8544_H

#include "component.h"
#include "itemlibrary.h"
#include "e-element.h"
#include "pin.h"

class MAINMODULE_EXPORT Pcd8544 : public Component, public eElement
{
    public:
        Pcd8544( QObject* parent, QString type, QString id );
        ~Pcd8544();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;
        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        void initPins();
        void clearLcd();
        void incrementPointer();
        void reset();
        void clearDDRAM();
        
        QImage *m_pdisplayImg;        //Visual representation of the LCD

        unsigned char m_aDispRam[6][84];                   //84x48 DDRAM

        //Controller state
        bool m_bPD;
        bool m_bV;
        bool m_bH;
        bool m_bD;
        bool m_bE;
        bool m_lastScl;
        int m_addrX;                                    // X RAM address
        int m_addrY;                                    // Y RAM address
        int m_inBit;        //How many bits have we read since last byte
        unsigned char m_cinBuf;     //Buffer where we keep incoming bits

        //Inputs
        Pin m_pRst;
        Pin m_pCs;
        Pin m_pDc;
        Pin m_pSi;
        Pin m_pScl;
};

#endif
