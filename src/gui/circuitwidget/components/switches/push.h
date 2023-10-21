/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PUSH_H
#define PUSH_H

#include "push_base.h"

class LibraryItem;

class Push : public PushBase
{
    public:
        Push( QString type, QString id );
        ~Push();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
