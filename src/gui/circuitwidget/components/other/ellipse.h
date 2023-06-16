/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "itemlibrary.h"
#include "shape.h"

class MAINMODULE_EXPORT Ellipse : public Shape
{
    public:
        Ellipse( QObject* parent, QString type, QString id );
        ~Ellipse();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
