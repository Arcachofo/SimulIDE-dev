/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BJT_H
#define BJT_H

#include "e-bjt.h"
#include "component.h"

class LibraryItem;

class BJT : public Component, public eBJT
{
    public:
        BJT( QString type, QString id );
        ~BJT();
        
 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        virtual void setPnp( bool pnp ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
};

#endif
