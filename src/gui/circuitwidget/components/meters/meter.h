/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef METER_H
#define METER_H

#include "e-resistor.h"
#include "component.h"

class IoPin;

class MAINMODULE_EXPORT Meter : public Component, public eResistor
{
    public:
        Meter( QObject* parent, QString type, QString id );
        ~Meter();

        virtual bool setPropStr( QString prop, QString val ) override;

        bool swithchPins() { return m_switchPins; }
        void setSwitchPins( bool s );

        virtual void initialize() override { m_crashed = false;}
        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual void setflip() override;

        QString m_unit;
        double m_dispValue;
        bool m_switchPins;

        IoPin* m_outPin;
        QGraphicsSimpleTextItem m_display;
};

#endif
