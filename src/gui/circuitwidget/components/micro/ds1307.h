/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        bool timeUpdtd() { return m_timeUpdtd; }
        void setTimeUpdtd( bool u ) { m_timeUpdtd = u; }

        virtual void initialize() override;
        virtual void stamp() override;

        virtual void writeByte() override;
        virtual void readByte() override;
        virtual void I2Cstop() override;

        char decToBcd(char val);
        char bcdToDec(char val);

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void updtTime();
        void updtDate();
        void updtCtrl();

        bool m_timeUpdtd;

        uint8_t m_addrPtr;
        uint8_t m_data[64];

        int m_phase = 0;

        IoPin* m_pinSda;
        IoPin* m_outpin;

        RtClock m_clock;
};

#endif
