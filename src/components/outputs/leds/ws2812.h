/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef WS2812_H
#define WS2812_H

#include "e-clocked_device.h"
#include "component.h"

class LibraryItem;

class WS2812 : public Component, public eClockedDevice
{
    public:
        WS2812( QString type, QString id );
        ~WS2812();

 static Component* construct( QString type, QString id );
 static LibraryItem *libraryItem();

        int  rows() { return m_rows; }
        void setRows( int rows );

        int  cols() { return m_cols; }
        void setCols( int cols );

        int resetPulse() { return m_RES; }
        void setResetPulse( int r ) { m_RES = r; }

        int t0H() { return m_T0H; }
        void setT0H( int t0h ) { m_T0H = t0h; }

        int t0L() { return m_T0L; }
        void setT0L( int t0l ) { m_T0L = t0l; }

        int t1H() { return m_T1H; }
        void setT1H( int t1h ) { m_T1H = t1h; }

        int t1L() { return m_T1L; }
        void setT1L( int t1l ) { m_T1L = t1l; }

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        void setOut( bool state );
        void saveBit( bool bit );
        void updateLeds();

        int m_state;
        int m_word;
        int m_byte;
        int m_bit;
        int m_data;

        int m_rows;
        int m_cols;
        int m_leds;

        uint64_t m_RES;
        uint64_t m_T0H;
        uint64_t m_T0L;
        uint64_t m_T1H;
        uint64_t m_T1L;

        double m_stepsPerus;

        bool m_lastHstate;
        bool m_newWord;

        uint64_t m_lastTime;

        std::vector<int> m_rgb;
        std::vector<QColor> m_led;

        IoPin* m_output;
};

#endif

