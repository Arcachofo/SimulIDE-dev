/***************************************************************************
 *   Copyright (C) 202 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TUNNEL_H
#define TUNNEL_H

#include "component.h"

class LibraryItem;
class eNode;
class Pin;

class MAINMODULE_EXPORT Tunnel : public Component
{
        Q_OBJECT
    public:
        Tunnel( QObject* parent, QString type, QString id );
        ~Tunnel();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();
        virtual bool setPropStr( QString prop, QString val ) override;

        QRectF boundingRect() const;

        QString name() { return m_name; }
        void setName( QString name );

        bool rotated() { return m_rotated; }
        void setRotated( bool rot );

        virtual void remove() override;

        virtual void registerEnode( eNode* enode, int n=-1 ) override;

        QString tunnelUid() { return m_tunUid; }
        void setTunnelUid( QString uid ) { m_tunUid = uid; }

        void setEnode( eNode* node );
        void setPacked( bool p ){ m_packed = p; }
        void removeTunnel();

        Pin* getPin() { return m_pin[0]; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

 static QHash<QString, eNode*> m_eNodes;

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

        int m_size;
        QString m_name;
        QString m_tunUid;

        bool m_rotated;
        bool m_blocked;
        bool m_packed;

 static QHash<QString, QList<Tunnel*>*> m_tunnels;
};

#endif
