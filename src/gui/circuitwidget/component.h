/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/


#ifndef COMPONENTITEM_H
#define COMPONENTITEM_H

#include <QtWidgets>
#include <QPointer>

struct property_t{
        QString name;
        QString caption;
        QString unit;
};

struct propGroup_t{
        QString name;
        QList<property_t> propList;
};

//Q_DECLARE_METATYPE( QList<int> )
//Q_DECLARE_METATYPE( QVector<int> )

class Pin;
class Label;
class PropDialog;

class MAINMODULE_EXPORT Component : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    Q_PROPERTY( QString  itemtype  READ itemType  USER  true )
    Q_PROPERTY( QString  id        READ idLabel   WRITE setIdLabel DESIGNABLE true USER true )
    Q_PROPERTY( bool     Show_id   READ showId    WRITE setShowId  DESIGNABLE true USER true )
    Q_PROPERTY( bool     Show_Val  READ showVal   WRITE setShowVal )
    Q_PROPERTY( QString  Unit      READ unit      WRITE setUnit )
    Q_PROPERTY( qreal    rotation  READ rotation  WRITE setRotation )
    Q_PROPERTY( int      x         READ x         WRITE setX )
    Q_PROPERTY( int      y         READ y         WRITE setY )
    Q_PROPERTY( QPointF  boardPos  READ boardPos  WRITE setBoardPos )
    Q_PROPERTY( QPointF  circPos   READ circPos   WRITE setCircPos )
    Q_PROPERTY( qreal    boardRot  READ boardRot  WRITE setBoardRot )
    Q_PROPERTY( qreal    circRot   READ circRot   WRITE setCircRot )
    Q_PROPERTY( int      labelx    READ labelx    WRITE setLabelX )
    Q_PROPERTY( int      labely    READ labely    WRITE setLabelY )
    Q_PROPERTY( int      labelrot  READ labelRot  WRITE setLabelRot )
    Q_PROPERTY( int      valLabelx READ valLabelx WRITE setValLabelX )
    Q_PROPERTY( int      valLabely READ valLabely WRITE setValLabelY )
    Q_PROPERTY( int      valLabRot READ valLabRot WRITE setValLabRot )
    Q_PROPERTY( int      hflip     READ hflip     WRITE setHflip )
    Q_PROPERTY( int      vflip     READ vflip     WRITE setVflip )
    Q_PROPERTY( bool     mainComp  READ isMainComp  WRITE setMainComp )

    public:
        QRectF boundingRect() const { return QRectF( m_area.x()-2, m_area.y()-2, m_area.width()+4 ,m_area.height()+4 ); }

        Component( QObject* parent, QString type, QString id );
        ~Component();

        virtual QList<propGroup_t> propGroups(){ QList<propGroup_t> pg; return pg;}

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QString idLabel();
        void setIdLabel( QString id );

        QString itemID();
        void setId( QString id );
        
        bool showId();
        void setShowId( bool show );
        
        bool showVal();
        void setShowVal( bool show );
        
        QString unit();
        virtual void setUnit( QString un );

        QPointF boardPos() { return m_boardPos; }
        void setBoardPos( QPointF p ) { m_boardPos = p; }

        QPointF circPos() { return m_circPos; }
        void setCircPos( QPointF p ) { m_circPos = p; }

        qreal boardRot() { return m_boardRot; }
        void setBoardRot( qreal rot ) { m_boardRot = rot; }

        qreal circRot() { return m_circRot; }
        void setCircRot( qreal rot ) { m_circRot = rot; }

        int labelx();
        void setLabelX( int x );

        int labely();
        void setLabelY( int y );

        int labelRot();
        void setLabelRot( int rot );
        
        void setLabelPos( int x, int y, int rot=0 );
        void setLabelPos();
        
        int valLabelx();
        virtual void setValLabelX( int x );

        int valLabely();
        virtual void setValLabelY( int y );

        int valLabRot();
        virtual void setValLabRot( int rot );
        
        int hflip();
        void setHflip( int hf );
        
        int vflip();
        void setVflip( int vf );
        
        void setValLabelPos( int x, int y, int rot );
        void setValLabelPos();
        
        void updateLabel( Label* label, QString txt );
        
        double getmultValue();

        QString backGround() { return m_BackGround; }
        virtual void setBackground( QString bck ){ m_BackGround = bck; }
        virtual void setSubcDir( QString dir ) { ; }
        
        void setPrintable( bool p );
        QString print();

        QString itemType();
        QString category();
        QIcon   icon();

        bool isGraphical() { return m_graphical; }
        bool isHidden() { return m_hidden; }
        virtual void setHidden( bool hid , bool hidLabel=false );

        virtual void inStateChanged( int ){}

        virtual void move( QPointF delta );
        void moveTo( QPointF pos );

 static bool m_selMainCo;
        bool isMainComp() { return m_mainComp; }
        void setMainComp( bool main ) { m_mainComp = main; }
        Component* getSubcircuit() { return m_subcircuit; }
        void setSubcircuit( Component* sc ) { m_subcircuit = sc; }

        virtual void remove();

        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    signals:
        void moved();

    public slots:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected slots:
        virtual void slotProperties();
        virtual void rotateCW();
        virtual void rotateCCW();
        virtual void rotateHalf();
        virtual void H_flip();
        virtual void V_flip();
        virtual void slotRemove();        void slotCopy();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event );

        void setValue( double val );
        void setflip();
        
        double m_value;

        const QString multUnits;
        double  m_unitMult;
        QString m_unit;
        QString m_mult;
        
        int m_Hflip;
        int m_Vflip;
 static int m_error;

        Label* m_idLabel;
        Label* m_valLabel;

        QString m_id;
        QString m_type;
        QString m_category;
        QString m_help;
        QString m_BackGround;   // BackGround Image
        
        QIcon   m_icon;
        QColor  m_color;
        QRectF  m_area;         // bounding rect
        QPointF m_eventpoint;

        QPointF m_boardPos;
        QPointF m_circPos;
        qreal   m_boardRot;
        qreal   m_circRot;

        bool m_showId;
        bool m_showVal;
        bool m_moving;
        bool m_printable;
        bool m_properties;
        bool m_hidden;
        bool m_graphical;
        bool m_mainComp;

        Component* m_subcircuit;

        PropDialog* m_propDialog;
        
        std::vector<Pin*> m_pin;
};

typedef Component* (*createItemPtr)( QObject* parent, QString type, QString id );


class Label : public QGraphicsTextItem
{
    friend class Component;
    
    Q_OBJECT
    public:
        Label( Component* parent );
        ~Label();

        void setLabelPos();

    public slots:
        void rotateCW();
        void rotateCCW();
        void rotate180();
        void H_flip( int hf );
        void V_flip( int vf );
        void updateGeometry(int, int, int);

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void focusOutEvent( QFocusEvent* event );

    private:
        Component* m_parentComp;
        
        int m_labelx;
        int m_labely;
        int m_labelrot;
};
#endif

