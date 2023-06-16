/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "itemlibrary.h"
#include "shape.h"

class MAINMODULE_EXPORT Rectangle : public Shape
{
    public:
        Rectangle( QObject* parent, QString type, QString id );
        ~Rectangle();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
