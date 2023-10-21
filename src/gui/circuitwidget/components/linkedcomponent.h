/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINKEDCOMPONENT_H
#define LINKEDCOMPONENT_H

#include "component.h"
#include "linkable.h"

class LinkedComponent : public Component, public Linkable
{
    public:
        LinkedComponent( QString type, QString id );
        ~LinkedComponent();

        void slotLinkComp() { Linkable::startLinking(); }

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;
};

#endif
