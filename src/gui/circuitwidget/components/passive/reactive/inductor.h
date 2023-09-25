/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "reactive.h"

class LibraryItem;

class MAINMODULE_EXPORT Inductor : public Reactive
{
    public:
        Inductor( QString type, QString id );
        ~Inductor();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        double indCurrent() { return m_curSource; }

        virtual void setCurrentValue( double c ) override;

        Pin* getPin( int n ) { return m_pin[n]; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual double updtRes()  override { return m_inductance/m_tStep; }
        virtual double updtCurr() override { return m_curSource - m_volt*m_admit; }

        double m_inductance;
};

#endif
