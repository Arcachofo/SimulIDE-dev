/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LED_H
#define LED_H

#include "ledbase.h"
#include "linkable.h"

class LibraryItem;

class Led : public LedBase, public Linkable
{
    public:
        Led( QString type, QString id );
        ~Led();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void voltChanged() override;

        void slotLinkComp() { Linkable::startLinking(); }

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;

        void drawBackground( QPainter* p ) override;
        void drawForeground( QPainter* p ) override;
};

#endif
