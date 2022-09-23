/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LED_H
#define LED_H

#include "ledbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Led : public LedBase
{
    public:
        Led( QObject* parent, QString type, QString id );
        ~Led();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

    protected:
        void drawBackground( QPainter* p );
        void drawForeground( QPainter* p );
};

#endif
