/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>

#include "linkercomponent.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "intprop.h"

#define tr(str) simulideTr("LinkerComponent",str)

LinkerComponent::LinkerComponent( QString type, QString id )
               : Component( type, id )
{
    m_isLinker = true;

    m_minVal = 0;
    m_maxVal = 0;

    addPropGroup( { "Hidden", {
        new StrProp<LinkerComponent>("Links", "Links",""
                                    , this, &LinkerComponent::getLinks , &LinkerComponent::setLinks )
    }, groupHidden} );
}
LinkerComponent::~LinkerComponent(){}

/*QList<ComProperty*> LinkerComponent::linkerProps()
{
    QList<ComProperty*> props =
    {
        //new ComProperty("", tr("Outputs:"),"","",0),

        new IntProp<LinkerComponent>("index", tr("Linker Index"), ""
                                     , this, &LinkerComponent::linkerIndex, &LinkerComponent::setLinkerIndex ),

        new DoubProp<LinkerComponent>("minVal", tr("Minimum Value"), ""
                                     , this, &LinkerComponent::minVal, &LinkerComponent::setMinVal ),

        new DoubProp<LinkerComponent>("maxVal", tr("Maximum value"), ""
                                     , this, &LinkerComponent::maxVal, &LinkerComponent::setMaxVal )
    };
    return props;
}*/

void LinkerComponent::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* linkCompAction = menu->addAction( QIcon(":/subcl.png"),tr("Link to Component") );
    QObject::connect( linkCompAction, &QAction::triggered, [=](){ slotLinkComp(); } );

    menu->addSeparator();

    Component::contextMenu( event, menu );
}
