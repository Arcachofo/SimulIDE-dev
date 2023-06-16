/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VARRESISTOR_H
#define VARRESISTOR_H

#include "varresbase.h"

class LibraryItem;

class MAINMODULE_EXPORT VarResistor : public VarResBase
{
    public:
        VarResistor( QObject* parent, QString type, QString id );
        ~VarResistor();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget *widget ) override;
};

#endif
