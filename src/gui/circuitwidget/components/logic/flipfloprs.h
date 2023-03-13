/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPRS_H
#define FLIPFLOPRS_H

#include "flipflopbase.h"

class LibraryItem;

class MAINMODULE_EXPORT FlipFlopRS : public FlipFlopBase
{
    public:
        FlipFlopRS( QObject* parent, QString type, QString id );
        ~FlipFlopRS();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void voltChanged() override;
};

#endif
