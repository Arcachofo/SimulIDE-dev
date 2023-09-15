/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QMenu>

#include "led.h"
#include "pin.h"
#include "itemlibrary.h"

#include "stringprop.h"

#define tr(str) simulideTr("Led",str)

Component* Led::construct( QString type, QString id )
{ return new Led( type, id ); }

LibraryItem* Led::libraryItem()
{
    return new LibraryItem(
        tr("Led"),
        "Leds",
        "led.png",
        "Led",
        Led::construct);
}

Led::Led( QString type, QString id )
   : LedBase( type, id )
{
    m_area = QRect(-8, -10, 20, 20 );

    m_linkable = true;

    m_pin.resize( 2 );
    m_pin[0] = new Pin( 180, QPoint(-16, 0 ), m_id+"-lPin", 0, this);
    m_pin[1] = new Pin(   0, QPoint( 16, 0 ), m_id+"-rPin", 1, this);
    m_pin[0]->setLength( 10 );

    setEpin( 0, m_pin[0] );
    setEpin( 1, m_pin[1] );

    addPropGroup( { "Hidden", {
new StrProp<Led>("Links", "Links","", this, &Led::getLinks , &Led::setLinks )
    }, groupHidden} );
}
Led::~Led(){}

void Led::voltChanged()
{
    eLed::voltChanged();
    if( !m_converged ) return;

    for( Component* comp : m_linkedComp ) comp->setLinkedValue( m_current );
}

void Led::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) { event->ignore(); return; }

    event->accept();
    QMenu* menu = new QMenu();

    QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
    QObject::connect( linkCompAction, &QAction::triggered, [=](){ slotLinkComp(); } );

    menu->addSeparator();

    Component::contextMenu( event, menu );
    menu->deleteLater();
}

void Led::drawBackground( QPainter* p )
{
    p->drawEllipse(-6,-8, 16, 16 );
}

void Led::drawForeground( QPainter* p )
{
    static const QPointF points[3] = {
        QPointF( 8, 0 ),
        QPointF(-3,-6 ),
        QPointF(-3, 6 ) };

    p->drawPolygon( points, 3 );
    p->drawLine( 8,-4, 8, 4 );
    p->drawLine(-6, 0, 10, 0 );
}
