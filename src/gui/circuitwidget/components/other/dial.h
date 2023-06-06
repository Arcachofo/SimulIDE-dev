/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIAL_H
#define DIAL_H

#include "dialed.h"
#include "dialwidget.h"
#include "linkable.h"

class LibraryItem;

class MAINMODULE_EXPORT Dial : public Dialed, public Linkable
{
    public:
        Dial( QObject* parent, QString type, QString id );
        ~Dial();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        bool slider() { return m_slider; }
        void setSlider( bool s );

        //virtual void compSelected( Component* comp ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void slotLinkComp() { Linkable::startLinking(); }

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;
        virtual void updateProxy() override;
};

#endif
