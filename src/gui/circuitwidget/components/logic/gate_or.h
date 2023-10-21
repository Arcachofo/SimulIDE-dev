/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ORGATE_H
#define ORGATE_H

#include "gate.h"
#include "component.h"

class LibraryItem;

class OrGate : public Gate
{
    public:
        OrGate( QString type, QString id );
        ~OrGate();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
        
    protected:
        virtual bool calcOutput( int inputs ) override;
};

#endif
