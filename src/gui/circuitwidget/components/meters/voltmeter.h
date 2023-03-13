/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTIMETER_H
#define VOLTIMETER_H

#include "meter.h"

class LibraryItem;

class MAINMODULE_EXPORT Voltimeter : public Meter
{
    public:
        Voltimeter( QObject* parent, QString type, QString id );
        ~Voltimeter();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void updateStep() override;
};

#endif
