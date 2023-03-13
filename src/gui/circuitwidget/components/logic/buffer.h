/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BUFFER_H
#define BUFFER_H

#include "gate.h"

class LibraryItem;

class MAINMODULE_EXPORT Buffer : public Gate
{
    public:
        Buffer( QObject* parent, QString type, QString id );
        ~Buffer();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void setTristate( bool t ) override;

        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
};

#endif
