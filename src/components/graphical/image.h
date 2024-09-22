/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IMAGE_H
#define IMAGE_H

#include "shape.h"

class LibraryItem;

class Image : public Shape
{
    public:
        Image( QString type, QString id );
        ~Image();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void setBackground( QString bck ) override;
        virtual QString background() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    public slots:
        void updateGif( const QRect &rect );
        void slotLoad();

    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        QPixmap m_image;
        QMovie* m_movie;
};

#endif
