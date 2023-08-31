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

class MAINMODULE_EXPORT Probe : public Component, public eElement
{
    public:
        Probe( QString type, QString id );
        ~Probe();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        void setVolt( double volt );

        double threshold() { return m_voltTrig; }
        void setThreshold( double t ) { m_voltTrig = t; }

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private: 
        double m_voltIn;
        double m_voltTrig;

        IoPin* m_inputPin;
};

#endif
