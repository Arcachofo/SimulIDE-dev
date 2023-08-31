/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SWITCH_H
#define SWITCH_H

#include "switch_base.h"

class LibraryItem;

class MAINMODULE_EXPORT Switch : public SwitchBase
{
    public:
        Switch( QString type, QString id );
        ~Switch();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        virtual void keyEvent( QString key, bool pressed ) override;
};

#endif
