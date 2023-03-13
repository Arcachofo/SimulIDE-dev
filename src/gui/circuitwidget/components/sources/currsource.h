/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CURRSOURCE_H
#define CURRSOURCE_H

#include "varsource.h"

class LibraryItem;
class Pin;

class MAINMODULE_EXPORT CurrSource : public VarSource
{
    public:
        CurrSource( QObject* parent, QString type, QString id );
        ~CurrSource();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;

    private:
        Pin* m_outPin;
};

#endif
