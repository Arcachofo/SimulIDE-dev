/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef GROUND_H
#define GROUND_H

#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT Ground : public Component
{
    public:
        Ground( QString type, QString id );
        ~Ground();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
