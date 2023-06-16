/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "capacitorbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Capacitor : public CapacitorBase
{
    public:
        Capacitor( QObject* parent, QString type, QString id );
        ~Capacitor();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
};

#endif
