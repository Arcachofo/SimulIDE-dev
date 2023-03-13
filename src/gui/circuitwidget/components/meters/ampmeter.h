/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AMPERIMETER_H
#define AMPERIMETER_H

#include "meter.h"

class LibraryItem;

class MAINMODULE_EXPORT Amperimeter : public Meter
{
    public:
        Amperimeter( QObject* parent, QString type, QString id );
        ~Amperimeter();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;
};

#endif
