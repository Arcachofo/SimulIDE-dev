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

        bool isBus();
        void setIsbus( bool b );

        bool rotated() { return m_rotated; }
        void setRotated( bool rot );

        virtual void remove() override;

        virtual void registerEnode( eNode* enode, int n=-1 ) override;

        QString tunnelUid() { return m_tunUid; }
        void setTunnelUid( QString uid ) { m_tunUid = uid; }

        void setEnode( eNode* node, int n=-1 );
        void setPacked( bool p );
        void removeTunnel();

        Pin* getPin() { return m_pin[0]; }
 static eNode* getEnode( QString n );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        void showGroup() { showHide( true ); }
        void hideGroup() { showHide( false ); }
        void setGroupName();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;

        void showHide( bool show );
        void setGroupName( QString name, bool single );

        int m_size;

        QString m_name;
        QString m_tunUid;

        bool m_rotated;
        bool m_blocked;
        bool m_packed;
        bool m_show;

 static QHash<QString, QList<Tunnel*>*> m_tunnels;
};

#endif
