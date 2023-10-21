/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINE_H
#define LINE_H

#include "itemlibrary.h"
#include "shape.h"

class Line : public Shape
{
    public:
        Line( QString type, QString id );
        ~Line();

 static Component* construct( QString type, QString id );
 static LibraryItem *libraryItem();

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
