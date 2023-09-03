/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QTextDocument>
#include <QApplication>
#include <QMenu>

#include "label.h"
#include "component.h"
#include "QDebug"

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
    float x = pos().x();
    float y = pos().y();

    QFontMetrics fm(this->font());
    QString text = toPlainText();
    int hf=hFlip();
    int vf=vFlip();
    if( m_labelrot == -90 ) {
        if( hf < 0 && vf < 0 ) {
            m_labelx = x - fm.height();
            m_labely = y + fm.horizontalAdvance( text );
        } else if( hf < 0 ) {
            m_labelx = x;
            m_labely = y + fm.horizontalAdvance( text );
        } else if( vf < 0 ) {
            m_labelx = x - fm.height();
            m_labely = y;
        } else {
            m_labelx = x;
            m_labely = y;
        }
    } else if( m_labelrot == 90 ) {
        if( hf < 0 && vf < 0 ) {
            m_labelx = x + fm.height();
            m_labely = y - fm.horizontalAdvance( text );
        } else if( hf < 0 ) {
            m_labelx = x;
            m_labely = y - fm.horizontalAdvance( text );
        } else if( vf < 0 ) {
            m_labelx = x + fm.height();
            m_labely = y;
        } else {
            m_labelx = x;
            m_labely = y;
        }
    } else {
        m_labelx = hf<0 ? x - fm.horizontalAdvance( text ) : x ;
        m_labely = vf<0 ? y - fm.height() : y ;
    }
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

        menu.exec(event->screenPos());
    }
}

int Label::hFlip()
{
    int val = 1;
    if( m_parentComp != nullptr ) val = ( m_parentComp->hflip()>0 )? val : -val;
    return val;
}

int Label::vFlip()
{
    int val = 1;
    if( m_parentComp != nullptr ) val = ( m_parentComp->vflip()>0 )? val : -val;
    return val;
}

void Label::setLabelPos( QPointF pos )
{
    m_labelx = pos.x();
    m_labely = pos.y();
}

void Label::updtLabelPos()
{
    QFontMetrics fm(this->font());
    QString text = toPlainText();
    bool vertical = ( m_labelrot == 90 || m_labelrot == -90 );
    int hf=hFlip();
    int vf=vFlip();
    int rotate = (vertical && ((vf<0) ^ (hf<0))) ? -m_labelrot : m_labelrot;
    if( m_labelrot == -90 ) {
        if( hf < 0 && vf < 0 ) {
            setX( m_labelx + fm.height() );
            setY( m_labely - fm.horizontalAdvance( text ) );
        } else if( hf < 0 ) {
            setX( m_labelx );
            setY( m_labely - fm.horizontalAdvance( text ) );
        } else if( vf < 0 ) {
            setX( m_labelx + fm.height() );
            setY( m_labely);
        } else {
            setX( m_labelx );
            setY( m_labely );
        }
    } else if( m_labelrot == 90 ) {
        if( hf < 0 && vf < 0 ) {
            setX( m_labelx - fm.height() );
            setY( m_labely + fm.horizontalAdvance( text ) );
        } else if( hf < 0 ) {
            setX( m_labelx );
            setY( m_labely + fm.horizontalAdvance( text ) );
        } else if( vf < 0 ) {
            setX( m_labelx - fm.height() );
            setY( m_labely );
        } else {
            setX( m_labelx );
            setY( m_labely );
        }
    } else {
        setX( hf<0 ? m_labelx + fm.horizontalAdvance( text ) : m_labelx );
        setY( vf<0 ? m_labely + fm.height() : m_labely );
    }
    //if( text == "R1") qDebug()<<" -- "<<pos()<<m_labelx<<m_labely;
    setRotation( rotate );
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

void Label::updateGeometry(int, int, int) { document()->setTextWidth( -1 ); }

