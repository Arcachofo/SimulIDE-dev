/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BJT_H
#define BJT_H

#include "e-bjt.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT BJT : public Component, public eBJT
{
    public:
        BJT( QObject* parent, QString type, QString id );
        ~BJT();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
};

#endif
