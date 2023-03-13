/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FULLADDER_H
#define FULLADDER_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT FullAdder : public LogicComponent
{
    public:
        FullAdder( QObject* parent, QString type, QString id );
        ~FullAdder();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }
};

#endif
