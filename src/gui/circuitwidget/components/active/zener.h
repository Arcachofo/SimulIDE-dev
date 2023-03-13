/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ZENER_H
#define ZENER_H

#include "diode.h"

class LibraryItem;

class MAINMODULE_EXPORT Zener : public Diode
{
    public:
        Zener( QObject* parent, QString type, QString id );
        ~Zener();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();
};

#endif
