/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CSOURCE_H
#define CSOURCE_H

#include "e-resistor.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT Csource : public Component, public eResistor
{
    public:
        Csource( QObject* parent, QString type, QString id );
        ~Csource();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

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

        //virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected slots:
        virtual void slotProperties() override;

    private:
        void udtProperties();

        double m_volt;
        double m_curr;
        double m_gain;

        bool m_controlPins;
        bool m_currSource;
        bool m_currControl;
};

#endif
