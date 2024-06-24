/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BUFFER_H
#define BUFFER_H

#include "gate.h"

class LibraryItem;

class Buffer : public Gate
{
    public:
        Buffer( QString type, QString id );
        ~Buffer();

        static Component* construct( QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void setTristate( bool t ) override;

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;
};

#endif
