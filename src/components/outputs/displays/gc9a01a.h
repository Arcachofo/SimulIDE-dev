/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "st77xx.h"

class LibraryItem;

class GC9A01A : public St77xx
{
    public:
        GC9A01A( QString type, QString id );
        ~GC9A01A();

        void setHeight( int h ) override;

        //void endTransaction() override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

    protected:
        //void displayReset() override;
};
