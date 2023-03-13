/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPJK_H
#define FLIPFLOPJK_H

#include "flipflopbase.h"

class LibraryItem;

class MAINMODULE_EXPORT FlipFlopJK : public FlipFlopBase
{
    public:
        FlipFlopJK( QObject* parent, QString type, QString id );
        ~FlipFlopJK();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

    protected:
        void calcOutput();
};

#endif
