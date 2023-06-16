/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RESISTOR_H
#define RESISTOR_H

#include "e-resistor.h"
#include "comp2pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Resistor : public Comp2Pin, public eResistor
{
    public:
        Resistor( QObject* parent, QString type, QString id );
        ~Resistor();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif

