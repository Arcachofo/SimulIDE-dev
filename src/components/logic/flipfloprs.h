/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPRS_H
#define FLIPFLOPRS_H

#include "flipflopbase.h"

class LibraryItem;

class FlipFlopRS : public FlipFlopBase
{
    public:
        FlipFlopRS( QString type, QString id );
        ~FlipFlopRS();

        static Component* construct( QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void voltChanged() override;
};

#endif
