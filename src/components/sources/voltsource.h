/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VOLTSOURCE_H
#define VOLTSOURCE_H

#include "varsource.h"

class LibraryItem;
class IoPin;

class VoltSource : public VarSource
{
    public:
        VoltSource( QString type, QString id );
        ~VoltSource();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void updateStep() override;

    private:
        IoPin* m_outPin;
};

#endif
