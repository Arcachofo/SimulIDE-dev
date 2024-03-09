/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RESISTOR_H
#define RESISTOR_H

#include "e-resistor.h"
#include "comp2pin.h"

class LibraryItem;

class Resistor : public Comp2Pin, public eResistor
{
    public:
        Resistor( QString type, QString id );
        ~Resistor();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;
};

#endif

