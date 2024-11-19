/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BUFFER_H
#define BUFFER_H

#include "gate.h"

class LibraryItem;

class Buffer : public Gate
{
    public:
        Buffer( QString type, QString id );
        ~Buffer();

        static Component* construct( QString type, QString id );
        static LibraryItem *libraryItem();

        void setTristate( bool t ) override;

    protected:
        void updatePath() override;
};

#endif
