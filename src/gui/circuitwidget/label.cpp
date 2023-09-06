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
    setPos( pos() + mapToItem( m_parentComp, event->pos() ) - mapToItem( m_parentComp, event->lastPos() ) );

    QFontMetrics fm( this->font() );
    QString text = toPlainText();

    QPointF p = adjustPos( pos().x(), pos().y(), -fm.horizontalAdvance( text ), -fm.height() );

    m_labelx = p.x();
    m_labely = p.y();
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

    int width  = fm.horizontalAdvance( text );
    int height = fm.height();

    QPointF p = adjustPos( m_labelx, m_labely, width, height );
    setPos( p );

    setTransformOriginPoint( width/2, height/2 );
    setRotation( m_labelrot*vFlip()*hFlip() );
    adjustSize();
}

QPointF Label::adjustPos( float x, float y, int width, int height )
{
    int hf=hFlip();
    int vf=vFlip();

    if( m_labelrot == -90 ) {
        if( vf < 0 ) x += height;
        if( hf < 0 ) y -= width;
    }
    else if( m_labelrot == 90 ){
        if( vf < 0 ) x -= height;
        if( hf < 0 ) y += width;
    }
    else {
        if( hf < 0 ) x += width;
        if( vf < 0 ) y += height;
    }
    return QPointF( x, y );
}

void Label::rotateCW()  { rotateAngle( 90 ); }
void Label::rotateCCW() { rotateAngle(-90 ); }
void Label::rotate180() { rotateAngle( 180); }

void Label::rotateAngle( double a )
{
    if( !isEnabled() ) return;
    m_labelrot += a*hFlip()*vFlip();
    updtLabelPos();
}

void Label::updateGeometry(int, int, int) { document()->setTextWidth( -1 ); }
