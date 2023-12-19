/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINKERCOMPONENT_H
#define LINKERCOMPONENT_H

#include "component.h"
#include "linker.h"

class LinkerComponent : public Component, public Linker
{
    public:
        LinkerComponent( QString type, QString id );
        ~LinkerComponent();

        void slotLinkComp() { Linker::startLinking(); }

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;
};

#endif
