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
        Inductor( QObject* parent, QString type, QString id );
        ~Inductor();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double indCurrent() { return m_curSource; }

        Pin* getPin( int n ) { return m_pin[n]; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual double updtRes()  override { return m_value/m_tStep; }
        virtual double updtCurr() override { return m_curSource - m_volt*m_admit; }
};

#endif
