/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ORGATE_H
#define ORGATE_H

#include "gate.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT OrGate : public Gate
{
        Q_OBJECT
    public:
        OrGate( QObject* parent, QString type, QString id );
        ~OrGate();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    protected:
        virtual bool calcOutput( int inputs ) override;
};

#endif
