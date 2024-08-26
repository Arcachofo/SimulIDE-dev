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

        int linkerIndex() { return m_linkerIndex; }
        void setLinkerIndex( int i ) { m_linkerIndex = i; }

        double minVal() { return m_minVal; }
        void setMinVal( double v ) { m_minVal = v; }

        double maxVal() { return m_maxVal; }
        void setMaxVal( double v ) { m_maxVal = v; }

        //QList<ComProperty*> linkerProps();

        void slotLinkComp() { Linker::startLinking(); }

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;
        //virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;

        int m_linkerIndex;

        double m_minVal;
        double m_maxVal;
};

#endif
