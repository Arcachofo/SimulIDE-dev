/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>

#include "linkedcomponent.h"

#include "stringprop.h"

#define tr(str) simulideTr("LinkedComponent",str)

LinkedComponent::LinkedComponent( QString type, QString id )
               : Component( type, id )
{
    m_linkable = true;

    addPropGroup( { "Hidden", {
new StrProp<LinkedComponent>("Links", "Links","", this, &LinkedComponent::getLinks , &LinkedComponent::setLinks )
    }, groupHidden} );
}
LinkedComponent::~LinkedComponent(){}

void LinkedComponent::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
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
