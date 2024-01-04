/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIAL_H
#define DIAL_H

#include "dialed.h"
#include "dialwidget.h"
#include "linker.h"

class LibraryItem;

class Dial : public Dialed, public Linker
{
    public:
        Dial( QString type, QString id );
        ~Dial();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

         int maxVal()  { return m_maxVal; }
         virtual void setMaxVal( int max );

         int minVal() { return m_minVal; }
         virtual void setMinVal( int min );

         int steps() { return m_steps; }
         void setSteps( int s );

        bool slider() { return m_slider; }
        void setSlider( bool s );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void slotLinkComp() { Linker::startLinking(); }

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;
        virtual void updateProxy() override;

        int m_minVal;
        int m_maxVal;
        int m_steps;
};

#endif
