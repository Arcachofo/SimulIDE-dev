/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef NODE_H
#define NODE_H

#include <QObject>

#include "component.h"
#include "pin.h"

class MAINMODULE_EXPORT Node : public Component, public QObject
{
    public:
        QRectF boundingRect() const { return QRect(-4,-4, 8, 8 ); }

        Node( QString type, QString id );
        ~Node();

        Pin* getPin( int pin ) const { return m_pin[pin]; }

        virtual void registerEnode( eNode* enode, int n=-1 ) override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        void inStateChanged( int rem=1 ) override;
        void checkRemove();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override {;}

    private:
        void joinConns( int co0, int c1);

        bool m_isBus;
};
#endif
