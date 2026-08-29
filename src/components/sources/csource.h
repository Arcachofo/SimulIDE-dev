/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "e-resistor.h"
#include "component.h"

class LibraryItem;

class Csource : public Component, public eResistor
{
    public:
        Csource( QString type, QString id );
        ~Csource();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        void stamp() override;
        void voltChanged() override;
        void updateStep() override;

        double gain() { return m_gain; }
        void setGain( double g );

        double volt() { return m_volt; }
        void setVolt( double volt );

        double outCurrent() { return m_curr; }
        void setOutCurrent( double c );

        bool currSource() { return m_currSource; }
        void setCurrSource( bool c );

        bool currControl() { return m_currControl; }
        void setCurrControl( bool c );

        bool controlPins() {return m_controlPins; }
        void setControlPins( bool set );

        void setVoltage( double v );

        virtual bool setLinkedTo( Linker* li ) override;
        virtual void setLinkedValue( double v, int i=0 ) override;

    protected:
        //virtual QPainterPath shape() const;
        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected slots:
        virtual void slotProperties() override;

    private:
        void updtProperties();

        double m_volt;
        double m_curr;
        double m_gain;

        double m_lastCurr;

        uint64_t m_lastTime;

        bool m_controlPins;
        bool m_currSource;
        bool m_currControl;
};
