/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPT_H
#define FLIPFLOPT_H

#include "flipflopbase.h"

class LibraryItem;

class MAINMODULE_EXPORT FlipFlopT : public FlipFlopBase
{
    public:
        FlipFlopT( QObject* parent, QString type, QString id );
        ~FlipFlopT();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

    protected:
        void calcOutput();
};

#endif
