/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QCursor>
#include <QMenu>

#include "packagepin.h"
#include "subpackage.h"
#include "utils.h"

PackagePin::PackagePin( int angle, const QPoint pos, QString id, int index, SubPackage* parent )
          : Pin( angle, pos, id, index, parent )
{
    m_area = QRectF(-1.5,-2, 10, 4);
    m_package = parent;

    setCursor( Qt::OpenHandCursor );
    setFlag( QGraphicsItem::ItemStacksBehindParent, false );

}
PackagePin::~PackagePin(){}

void PackagePin::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        event->accept();
        setCursor( Qt::ClosedHandCursor );
    }
}

void PackagePin::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();

    QPointF delta(0,0);
    delta = toGrid(event->scenePos()) - toGrid(event->lastScenePos());
    if( !(fabs( delta.x() )> 0) && !(fabs( delta.y() )> 0) ) return;

    moveBy( delta.x(), delta.y() );
    if( !this->collidesWithItem( m_component ) ) moveBy( -delta.x(), -delta.y() );
}

void PackagePin::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setCursor( Qt::OpenHandCursor );
}

void PackagePin::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    m_package->setEventPin( this );

    QMenu* menu = new QMenu();
    QAction* editAction = menu->addAction( QIcon(":/rename.svg"),QObject::tr("Edit Pin "));
    QObject::connect( editAction, &QAction::triggered, m_package, &SubPackage::editPin, Qt::UniqueConnection );

    QAction* deleteAction = menu->addAction( QIcon(":/remove.svg"),QObject::tr("Delete Pin ") );
    QObject::connect( deleteAction, &QAction::triggered, m_package, &SubPackage::deleteEventPin, Qt::UniqueConnection );

    menu->exec( event->screenPos() );
}

void PackagePin::setLabelText( QString label, bool over )
{
    Pin::setLabelText( label, over );
    setFlag( QGraphicsItem::ItemStacksBehindParent, false );

}

void PackagePin::setLength( int length )
{
    Pin::setLength( length );
    m_area = QRectF(-1.5,-2, m_length+2, 4 );
}

void PackagePin::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Pin::paint( p, option, widget );
    QPen pen( Qt::lightGray, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );

    p->drawLine(-1, 0, 1, 0 );
    p->drawLine( 0,-1, 0, 1 );
}
