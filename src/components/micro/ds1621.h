/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "twimodule.h"
#include "iocomponent.h"
#include "pin.h"

class LibraryItem;

class DS1621 : public IoComponent, public TwiModule
{
    public:
        DS1621( QString type, QString id );
        ~DS1621();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc );

        double temp() { return m_temp; }
        void setTemp( double temp );

        void stamp() override;
        void runEvent() override;
        void voltChanged() override;

        void readByte() override;
        void writeByte() override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

        void upbuttonclicked();
        void downbuttonclicked();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

    public slots:
        void slotOpenMon();

    protected:
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

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

        Pin m_vdd;
        Pin m_gnd;
};
