/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include "compbase.h"
#include "updatable.h"

class Pin;
class eNode;
class Label;

class MAINMODULE_EXPORT Component : public CompBase, public QGraphicsItem, public Updatable
{
    Q_OBJECT
    Q_INTERFACES( QGraphicsItem )

    public:
        QRectF boundingRect() const { return QRectF( m_area.x()-2, m_area.y()-2, m_area.width()+4, m_area.height()+4 ); }

        Component( QObject* parent, QString type, QString id );
        ~Component();

        virtual bool setPropStr( QString prop, QString val ) override;
 static void substitution( QString &propName );

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QPointF position() { return pos(); }
        void setPosition( QPointF pos ) { setPos( pos ); }

        double getAngle() { return rotation(); }
        void setAngle( double angle ) { setRotation( angle ); }

        QString idLabel();
        void setIdLabel( QString id );

        bool showId() { return m_showId; }
        void setShowId( bool show );

        QPointF getIdPos();
        void setIdPos( QPointF p );

        int getIdRot();
        void setIdRot( int r );

        void setLabelPos( int x, int y, int rot=0 );
        void updtLabelPos();

        bool showVal() { return m_showVal; }
        void setShowVal( bool show );

        QPointF getValPos();
        void    setValPos( QPointF p );

        int  getValRot();
        void setValRot( int r );

        void setValLabelPos( int x, int y, int rot );
        void updtValLabelPos();

        virtual void setValLabelText( QString t ) override;

        virtual QString showProp() override;
        virtual void    setShowProp( QString prop )override;

        bool isGraphical() { return m_graphical; }

        bool isMainComp() { return m_isMainComp; }
        void setMainComp( bool m ) { m_isMainComp = m; }

        QPointF boardPos() { return m_boardPos; }
        void    setBoardPos( QPointF pos ) { m_boardPos = pos; }

        QPointF circPos() { return m_circPos; }
        void    setCircPos( QPointF pos ) { m_circPos = pos; }

        double boardRot() { return m_boardRot; }
        void   setBoardRot( double rot ) { m_boardRot = rot; }

        double circRot() { return m_circRot; }
        void   setCircRot( double rot ) { m_circRot = rot; }

        int  hflip() { return m_Hflip; }
        virtual void setHflip( int hf );

        int  vflip() { return m_Vflip; }
        void setVflip( int vf );

        QString print();

        bool isHidden() { return m_hidden; }
        virtual void setHidden( bool hid, bool hidLabel=false );

        virtual void setBackground( QString bck ) { m_background = bck;}
        //virtual void setSubcDir( QString dir ) {;}

        virtual void registerEnode( eNode*, int n=-1 ) {;}

        virtual void updtValues() {;}

        virtual void inStateChanged( int ){;}

        virtual void move( QPointF delta ) { setPos( pos() + delta ); emit moved(); }
        void moveTo( QPointF pos ){ setPos( pos ); emit moved(); }

        virtual void remove();

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* );

        //Component* m_subcircuit;

        bool m_printable;

    signals:
        void moved();
        void flip( int h, int v );

    public slots:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected slots:
        virtual void slotProperties();
        void rotateCW();
        void rotateCCW();
        void rotateHalf();
        void H_flip();
        void V_flip();
        void slotRemove();
        void slotGroup();
        void slotCopy();
        void slotCut();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event );

        virtual void setflip();

        bool m_graphical;
        bool m_showId;
        bool m_showVal;
        bool m_moving;
        bool m_warning;
        bool m_crashed;
        bool m_hidden;

        QPointF m_boardPos;
        QPointF m_circPos;
        double  m_boardRot;
        double  m_circRot;

        QString m_showProperty; // Property shown in val Label

        double  m_opCount;
        
        int m_Hflip;
        int m_Vflip;
 static int m_error;
 static bool m_selMainCo;

        QString m_help;
        QString m_background;   // BackGround Image path

        QColor  m_color;
        QRectF  m_area;         // bounding rect
        QPointF m_eventpoint;

        Label* m_idLabel;
        Label* m_valLabel;

        std::vector<Pin*> m_pin;
};

typedef Component* (*createItemPtr)( QObject* parent, QString type, QString id );

#endif
