/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPT_H
#define FLIPFLOPT_H

#include "flipflopbase.h"

class LibraryItem;

class FlipFlopT : public FlipFlopBase
{
    public:
        FlipFlopT( QString type, QString id );
        ~FlipFlopT();

        static Component* construct( QString type, QString id );
        static LibraryItem *libraryItem();

    protected:
        void calcOutput();
};

#endif
