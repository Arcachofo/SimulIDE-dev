/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef WS2812_H
#define WS2812_H

#include "e-clocked_device.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT WS2812 : public Component, public eClockedDevice
{
    public:
        WS2812( QObject* parent, QString type, QString id );
        ~WS2812();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        int  rows() { return m_rows; }
        void setRows( int rows );

        int  cols() { return m_cols; }
        void setCols( int cols );

        virtual void initialize() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

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

        double m_stepsPerus;

        bool m_lastHstate;
        bool m_newWord;

        uint64_t m_lastTime;

        std::vector<int> m_rgb;
        std::vector<QColor> m_led;

        IoPin* m_output;
};

#endif

