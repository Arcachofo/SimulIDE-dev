/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGuiApplication>
#include <QCursor>

#include "linkable.h"
#include "component.h"
#include "circuit.h"

Linkable::Linkable(){}
Linkable::~Linkable(){}

QString Linkable::getLinks()
{
    QString links;

    for( int i=0; i<m_linkedComp.size(); ++i )
    {
        Component* comp = m_linkedComp.at( i );
        links.append( comp->getUid()+"," );
    }
    return links;
}

void Linkable::setLinks( QString links )
{
    m_linkedStr = links;
}

void Linkable::createLinks( QList<Component*>* compList )
{
    QStringList components = m_linkedStr.split(",");
    for( QString uid : components )
    {
        if( uid.isEmpty() ) continue;

        for( Component* comp : *compList )
            if( comp->getUid().contains( uid ) )
            {
                //qDebug() << "TextComponent::createLinks"<<uid;
                m_linkedComp.append( comp );
                comp->setLinked( true );
                break;
            }
    }
}

void Linkable::startLinking()  // Start linking Components
{
    if( Component::m_selecComp ) Component::m_selecComp->compSelected( NULL ); // Finish previous linking
    Component::m_selecComp = this;

    for( int i=0; i<m_linkedComp.size(); ++i ) // Set numbers for visualization
    {
        Component* comp = m_linkedComp.at( i );
        comp->m_linkNumber = i;
        comp->update();
    }
    QGuiApplication::setOverrideCursor( Qt::PointingHandCursor );
    Circuit::self()->update();
}

void Linkable::compSelected( Component* comp )
{
    if( comp )  // One Component was selected to link
    {
        bool linked = m_linkedComp.contains( comp );
        comp->setLinked( !linked );

        if( linked )
        {
            comp->m_linkNumber = -1;
            m_linkedComp.removeAll( comp );
        }
        else m_linkedComp.append( comp );

        for( int i=0; i<m_linkedComp.size(); ++i )
        {
            Component* comp = m_linkedComp.at( i );
            comp->m_linkNumber = i;
        }
    }
    else       // End of linking Components
    {
        Component::m_selecComp = NULL;

        for( Component* comp : m_linkedComp )  // Clear numbers for visualization
            comp->m_linkNumber = -1;

        Circuit::self()->update();
    }
}
