/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SOCKET_H
#define SOCKET_H

#include "connbase.h"

class LibraryItem;

class Socket : public ConnBase
{
    public:
        Socket( QString type, QString id );
        ~Socket();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        void updatePins() override { connectPins( false ); }
        void connectPins( bool connect );

    private:
        virtual void updatePixmap() override;
};

#endif
