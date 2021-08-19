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

#include "label.h"
#include "updatable.h"

struct property_t{
        QString name;
        QString caption;
        QString unit;
};

struct propGroup_t{
        QString name;
        QList<property_t> propList;
};

class Pin;
class PropDialog;

class MAINMODULE_EXPORT Component : public QObject, public QGraphicsItem, public Updatable
{
    Q_OBJECT
    Q_INTERFACES( QGraphicsItem )

    Q_PROPERTY( QString  itemtype  READ itemType  USER  true )
    Q_PROPERTY( QString  id        READ idLabel   WRITE setIdLabel DESIGNABLE true USER true )
    Q_PROPERTY( bool     Show_id   READ showId    WRITE setShowId  DESIGNABLE true USER true )
    Q_PROPERTY( bool     Show_Val  READ showVal   WRITE setShowVal )
    Q_PROPERTY( QString  Unit      READ unit      WRITE setUnit )
    Q_PROPERTY( qreal    rotation  READ rotation  WRITE setRotation )
    Q_PROPERTY( int      x         READ x         WRITE setX )
    Q_PROPERTY( int      y         READ y         WRITE setY )
    Q_PROPERTY( int      labelx    READ labelx    WRITE setLabelX )
    Q_PROPERTY( int      labely    READ labely    WRITE setLabelY )
    Q_PROPERTY( int      labelrot  READ labelRot  WRITE setLabelRot )
    Q_PROPERTY( int      valLabelx READ valLabelx WRITE setValLabelX )
    Q_PROPERTY( int      valLabely READ valLabely WRITE setValLabelY )
    Q_PROPERTY( int      valLabRot READ valLabRot WRITE setValLabRot )
    Q_PROPERTY( int      hflip     READ hflip     WRITE setHflip )
    Q_PROPERTY( int      vflip     READ vflip     WRITE setVflip )
    // Related to Subcircuit:
    Q_PROPERTY( bool     mainComp  READ isMainComp  WRITE setMainComp )
        // Hack: SCRIPTABLE is used to identify Board properties
    Q_PROPERTY( QPointF  boardPos  READ boardPos  WRITE setBoardPos SCRIPTABLE false )
    Q_PROPERTY( QPointF  circPos   READ circPos   WRITE setCircPos  SCRIPTABLE false )
    Q_PROPERTY( qreal    boardRot  READ boardRot  WRITE setBoardRot SCRIPTABLE false )
    Q_PROPERTY( qreal    circRot   READ circRot   WRITE setCircRot  SCRIPTABLE false )

    public:
        QRectF boundingRect() const { return QRectF( m_area.x()-2, m_area.y()-2, m_area.width()+4 ,m_area.height()+4 ); }

        Component( QObject* parent, QString type, QString id );
        ~Component();

        enum trigger_t {
            None = 0,
            Clock,
            InEnable
        };
        Q_ENUM( trigger_t ) // We need this here bcos eClockedDevice is not a QObject

        virtual QList<propGroup_t> propGroups(){ QList<propGroup_t> pg; return pg;}
        QStringList userProperties();

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QString idLabel() { return m_idLabel->toPlainText(); }
        void setIdLabel( QString id ) { m_idLabel->setPlainText( id ); }

        QString itemID() { return  m_id; }
        void setId( QString id ) {  m_id = id;  }
        
        bool showId() { return m_showId; }
        void setShowId( bool show ) { m_idLabel->setVisible( show ); m_showId = show; }
        
        bool showVal() { return m_showVal; }
        void setShowVal( bool show ) { m_valLabel->setVisible( show ); m_showVal = show; }
        
        QString unit() { return m_mult+m_unit; }
        virtual void setUnit( QString un );

        QPointF boardPos() { return m_boardPos; }
        void setBoardPos( QPointF p ) { m_boardPos = p; }

        QPointF circPos() { return m_circPos; }
        void setCircPos( QPointF p ) { m_circPos = p; }

        qreal boardRot() { return m_boardRot; }
        void setBoardRot( qreal rot ) { m_boardRot = rot; }

        qreal circRot() { return m_circRot; }
        void setCircRot( qreal rot ) { m_circRot = rot; }

        int labelx() { return m_idLabel->m_labelx; }
        void setLabelX( int x ) { m_idLabel->m_labelx = x; }

        int labely() { return m_idLabel->m_labely; }
        void setLabelY( int y ) { m_idLabel->m_labely = y; }

        int labelRot() { return m_idLabel->m_labelrot; }
        void setLabelRot( int rot ) { m_idLabel->m_labelrot = rot; }
        
        void setLabelPos( int x, int y, int rot=0 );
        void setLabelPos() { m_idLabel->setLabelPos(); }
        
        int valLabelx() { return m_valLabel->m_labelx; }
        virtual void setValLabelX( int x )  { m_valLabel->m_labelx = x; }

        int valLabely() { return m_valLabel->m_labely; }
        virtual void setValLabelY( int y ) { m_valLabel->m_labely = y; }

        int valLabRot() { return m_valLabel->m_labelrot; }
        virtual void setValLabRot( int rot ) { m_valLabel->m_labelrot = rot; }
        
        int hflip() { return m_Hflip; }
        void setHflip( int hf );
        
        int vflip() { return m_Vflip; }
        void setVflip( int vf );
        
        void setValLabelPos( int x, int y, int rot );
        void setValLabelPos() { m_valLabel->setLabelPos(); }
        
        void updateLabel( Label* label, QString txt );
        
        // double getmultValue() { return m_value*m_unitMult; }

        QString backGround() { return m_BackGround; }
        virtual void setBackground( QString bck ){ m_BackGround = bck; }
        virtual void setSubcDir( QString dir ) { ; }
        
        void setPrintable( bool p ) { m_printable = p; }
        QString print();

        QString itemType() { return m_type; }
        QString category() { return m_category; }
        QIcon   icon() { return m_icon; }

        bool isGraphical() { return m_graphical; }
        bool isHidden() { return m_hidden; }
        virtual void setHidden( bool hid , bool hidLabel=false );

        virtual void inStateChanged( int ){}

        virtual void move( QPointF delta ) { setPos( pos() + delta ); emit moved(); }
        void moveTo( QPointF pos ){ setPos( pos ); emit moved(); }

 static bool m_selMainCo;
        bool isMainComp() { return m_mainComp; }
        void setMainComp( bool main ) { m_mainComp = main; }
        Component* getSubcircuit() { return m_subcircuit; }
        void setSubcircuit( Component* sc ) { m_subcircuit = sc; }

        virtual void remove();

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* );

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
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
            { if( event->button() == Qt::LeftButton ) slotProperties(); }

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
        bool m_crashed;

        Component* m_subcircuit;

        PropDialog* m_propDialog;
        
        std::vector<Pin*> m_pin;
};

typedef Component* (*createItemPtr)( QObject* parent, QString type, QString id );

#endif

