/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTSOURCE_H
#define VOLTSOURCE_H

#include "varsource.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT VoltSource : public VarSource
{
    public:
        VoltSource( QObject* parent, QString type, QString id );
        ~VoltSource();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void updateStep() override;

    private:
        IoPin* m_outPin;
};

#endif
