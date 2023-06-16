/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IMAGE_H
#define IMAGE_H

#include "shape.h"

class LibraryItem;

class MAINMODULE_EXPORT Image : public Shape
{
    public:
        Image( QObject* parent, QString type, QString id );
        ~Image();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void setBackground( QString bck ) override;
        QString background();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

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
