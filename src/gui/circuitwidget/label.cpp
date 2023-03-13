/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QTextDocument>
#include <QApplication>
#include <QMenu>

#include "label.h"
#include "component.h"

Label::Label()
     : QGraphicsTextItem()
{
    m_labelrot = 0;
    setCursor( Qt::OpenHandCursor );

    connect( document(), &QTextDocument::contentsChange,
             this,       &Label::updateGeometry, Qt::UniqueConnection );
}
Label::~Label() { }

void Label::setComponent( Component* parent )
{
    m_parentComp = parent;
    this->setParentItem( parent );
    this->document()->setDocumentMargin(0);
}

void Label::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        event->accept();
        setCursor( Qt::ClosedHandCursor );
        grabMouse();
}   }

void Label::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setPos(  pos() + mapToItem( m_parentComp, event->pos() ) - mapToItem( m_parentComp, event->lastPos() ) );
    m_labelx = int(pos().x());
    m_labely = int(pos().y());
}

void Label::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setCursor( Qt::OpenHandCursor );
    ungrabMouse();
}

void Label::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu menu;

        QAction* rotateCWAction = menu.addAction(QIcon(":/rotatecw.svg"),"Rotate CW");
        connect(rotateCWAction, &QAction::triggered,
                          this, &Label::rotateCW, Qt::UniqueConnection );

        QAction* rotateCCWAction = menu.addAction(QIcon(":/rotateccw.svg"),"Rotate CCW");
        connect(rotateCCWAction, &QAction::triggered,
                           this, &Label::rotateCCW, Qt::UniqueConnection );

        QAction* rotate180Action = menu.addAction(QIcon(":/rotate180.svg"),"Rotate 180º");
        connect(rotate180Action, &QAction::triggered,
                           this, &Label::rotate180, Qt::UniqueConnection );

        /*QAction* selectedAction = */menu.exec(event->screenPos());
    }
}

void Label::setLabelPos( QPointF pos )
{
    m_labelx = pos.x();
    m_labely = pos.y();
}

void Label::updtLabelPos()
{
    setX( m_labelx );
    setY( m_labely );
    setRotation( m_labelrot );
    adjustSize();
}

void Label::rotateCW()
{
    if( !isEnabled() ) return;
    setRotation( rotation() + 90 );
    m_labelrot = int(rotation()) ;
}

void Label::rotateCCW()
{
    if( !isEnabled() ) return;
    setRotation( rotation() - 90 );
    m_labelrot = int(rotation()) ;
}

void Label::rotate180()
{
    if( !isEnabled() ) return;
    setRotation( rotation() - 180 );
    m_labelrot = int(rotation()) ;
}

void Label::H_flip( int hf )
{
    if( !isEnabled() ) return;
    setTransform( QTransform::fromScale(hf, 1) );
}

void Label::V_flip( int vf )
{
    if( !isEnabled() ) return;
    setTransform( QTransform::fromScale(1, vf) );
}
void Label::updateGeometry(int, int, int) { document()->setTextWidth( -1 ); }

