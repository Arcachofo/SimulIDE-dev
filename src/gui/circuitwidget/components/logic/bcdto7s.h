/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BCDTO7S_H
#define BCDTO7S_H

#include "bcdbase.h"

class LibraryItem;

class MAINMODULE_EXPORT BcdTo7S : public BcdBase
{
    public:
        BcdTo7S( QObject* parent, QString type, QString id );
        ~BcdTo7S();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }
};

#endif
