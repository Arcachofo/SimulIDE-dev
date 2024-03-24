/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LAMP_H
#define LAMP_H

#include "e-resistor.h"
#include "comp2pin.h"

class LibraryItem;

class Lamp : public Comp2Pin, public eResistor
{
    public:
        Lamp( QString type, QString id );
        ~Lamp();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        double maxCurrent()             { return m_maxCurrent; }
        void  setMaxCurrent( double c ) { m_maxCurrent = c; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        virtual void updateVI() override;
        void updateBright();
        double overCurrent() { return m_avgCurrent/m_maxCurrent; }

        uint64_t m_prevStep;
        uint32_t m_intensity;

        double m_maxCurrent;
        double m_avgCurrent;
        double m_totalCurrent;
        double m_lastCurrent;
        double m_lastPeriod;
        double m_brightness;
};

#endif
