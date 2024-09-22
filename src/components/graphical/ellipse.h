/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "shape.h"

class LibraryItem;

class Ellipse : public Shape
{
    public:
        Ellipse( QString type, QString id );
        ~Ellipse();
        
 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;
};

#endif
