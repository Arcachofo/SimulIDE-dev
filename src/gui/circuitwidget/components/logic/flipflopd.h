/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPD_H
#define FLIPFLOPD_H

#include "flipflopbase.h"

class LibraryItem;

class MAINMODULE_EXPORT FlipFlopD : public FlipFlopBase
{
    public:
        FlipFlopD( QObject* parent, QString type, QString id );
        ~FlipFlopD();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

    protected:
        void calcOutput();
};

#endif
