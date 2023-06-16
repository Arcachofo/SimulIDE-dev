/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SOCKET_H
#define SOCKET_H

#include "connbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Socket : public ConnBase
{
    public:
        Socket( QObject* parent, QString type, QString id );
        ~Socket();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        void updatePins( bool connect );

    private:
        virtual void updatePixmap() override;
};

#endif
