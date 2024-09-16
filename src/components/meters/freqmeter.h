/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FREQMETER_H
#define FREQMETER_H

#include "component.h"
#include "e-element.h"

class LibraryItem;

class FreqMeter : public Component, public eElement
{
    public:
        FreqMeter( QString type, QString id );
        ~FreqMeter();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        double filter() { return m_filter; }
        void setFilter( double f ) { m_filter = f; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;
        
    protected:
        virtual void setflip() override;

        bool m_rising;
        bool m_falling;
        
        double m_filter;
        double m_lastData;
        double m_freq;
        
        int m_numMax;

        uint64_t m_lastMax;
        uint64_t m_totalP;
        uint64_t m_period;
        
        QGraphicsSimpleTextItem m_display;
};

#endif
