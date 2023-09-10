/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGuiApplication>
#include <QCursor>

#include "linkable.h"
#include "component.h"
#include "circuit.h"

Linkable* Linkable::m_selecComp = NULL;

Linkable::Linkable()
        : m_linkCursor( QPixmap(":/link.png"), 10, 10 )
{}
Linkable::~Linkable()
{
    if( m_selecComp == this ) Linkable::compSelected( NULL ); // Cancel link to components
}

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

void Linkable::createLinks( QSet<Component*>* compList )
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
    if( m_selecComp ) m_selecComp->compSelected( NULL ); // Finish previous linking
    m_selecComp = this;

    for( int i=0; i<m_linkedComp.size(); ++i ) // Set numbers for visualization
    {
        Component* comp = m_linkedComp.at( i );
        comp->m_linkNumber = i;
        comp->update();
    }
    QGuiApplication::setOverrideCursor( m_linkCursor ); // ( Qt::PointingHandCursor );
    Circuit::self()->update();
}

void Linkable::stopLinking() // Static
{
     if( !m_selecComp ) return;
     m_selecComp->compSelected( NULL );
     m_selecComp = NULL;

     QGuiApplication::restoreOverrideCursor();
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
        m_selecComp = NULL;

        for( Component* comp : m_linkedComp )  // Clear numbers for visualization
            comp->m_linkNumber = -1;

        Circuit::self()->update();
    }
}
