/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "label.h"
#include "component.h"

Label::Label()
     : QGraphicsTextItem()
{
    m_labelrot = 0;
    setCursor( Qt::OpenHandCursor );

    this->document()->setDocumentMargin(0);

    connect( document(), SIGNAL( contentsChange(int, int, int)),
             this,       SLOT(   updateGeometry(int, int, int)), Qt::UniqueConnection );
}
Label::~Label() { }

void Label::setComponent( Component* parent )
{
    m_parentComp = parent;
    this->setParentItem( parent );
}

void Label::focusOutEvent( QFocusEvent* event )
{
    setTextInteractionFlags( Qt::NoTextInteraction );
    m_parentComp->updateLabel( this, document()->toPlainText() );

    QGraphicsTextItem::focusOutEvent(event);
}

void Label::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        event->accept();
        setCursor( Qt::ClosedHandCursor );
        grabMouse();
    }
}

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

        QAction* rotateCWAction = menu.addAction(QIcon(":/rotateCW.png"),"Rotate CW");
        connect(rotateCWAction, SIGNAL(triggered()),
                          this, SLOT(rotateCW()), Qt::UniqueConnection );

        QAction* rotateCCWAction = menu.addAction(QIcon(":/rotateCCW.png"),"Rotate CCW");
        connect(rotateCCWAction, SIGNAL(triggered()),
                           this, SLOT(rotateCCW()), Qt::UniqueConnection );

        QAction* rotate180Action = menu.addAction(QIcon(":/rotate180.png"),"Rotate 180º");
        connect(rotate180Action, SIGNAL(triggered()),
                           this, SLOT(rotate180()), Qt::UniqueConnection );

        /*QAction* selectedAction = */menu.exec(event->screenPos());
    }
}

void Label::setLabelPos()
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

//#include "moc_label.h"
