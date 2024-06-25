/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PROBE_H
#define PROBE_H

#include "component.h"
#include "e-element.h"

class IoPin;
class LibraryItem;

class Probe : public Component, public eElement
{
    public:
        Probe( QString type, QString id );
        ~Probe();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        void setVolt( double volt );

        void setSmall( bool s );
        bool isSmall() { return m_small; }

        double threshold() { return m_voltTrig; }
        void setThreshold( double t ) { m_voltTrig = t; }

        virtual void rotateAngle( double a ) override;

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private: 
        double m_voltIn;
        double m_voltTrig;

        bool m_small;

        IoPin* m_inputPin;
};

#endif
