/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>

#include "linkercomponent.h"

#include "stringprop.h"

#define tr(str) simulideTr("LinkerComponent",str)

LinkerComponent::LinkerComponent( QString type, QString id )
               : Component( type, id )
{
    m_linker = true;

    addPropGroup( { "Hidden", {
new StrProp<LinkerComponent>("Links", "Links","", this, &LinkerComponent::getLinks , &LinkerComponent::setLinks )
    }, groupHidden} );
}
LinkerComponent::~LinkerComponent(){}

void LinkerComponent::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) { event->ignore(); return; }

    event->accept();
    QMenu* menu = new QMenu();

    if( !parentItem() )
    {
        QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
        QObject::connect( linkCompAction, &QAction::triggered, [=](){ slotLinkComp(); } );

        menu->addSeparator();
    }
    Component::contextMenu( event, menu );
    menu->deleteLater();
}
