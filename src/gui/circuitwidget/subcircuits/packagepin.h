/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PACKAGEPIN_H
#define PACKAGEPIN_H

#include "pin.h"

class SubPackage;
class PackagePin : public Pin
{
    public:
        PackagePin( int angle, const QPoint pos, QString id, int index, SubPackage* parent=0 );
        ~PackagePin();

        virtual void setLabelText( QString label, bool over=true ) override;
        virtual void setLength( int length ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
        virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
        virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;

    private:
        SubPackage* m_package;
};
#endif
