/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGuiApplication>
#include <QCursor>

#include "linker.h"
#include "component.h"
#include "circuit.h"

Linker* Linker::m_selecComp = nullptr;

Linker::Linker()
      : m_linkCursor( QPixmap(":/link.png"), 10, 10 )
{}
Linker::~Linker()
{
    if( m_selecComp == this ) Linker::compSelected( nullptr ); // Cancel link to components
    for( Component* comp : m_linkedComp ) comp->setLinkedTo( nullptr );
}

QString Linker::getLinks()
{
    QString links;

    for( Component* comp : m_linkedComp ) links.append( comp->getUid()+"," );

    return links;
}

void Linker::setLinks( QString links )
{
    m_linkedStr = links;
}

void Linker::createLinks( QList<Component*>* compList )
{
    QStringList components = m_linkedStr.split(",");
    for( QString uid : components )
    {
        if( uid.isEmpty() ) continue;

        for( Component* comp : *compList )
            if( comp->getUid().contains( uid ) )
            {
                if( comp->setLinkedTo( this ) ) m_linkedComp.append( comp );
                break;
            }
    }
}

void Linker::startLinking()  // Start linking Components
{
    if( m_selecComp ) m_selecComp->compSelected( nullptr ); // Finish previous linking
    m_selecComp = this;

    showLinked( true );

    QGuiApplication::setOverrideCursor( m_linkCursor ); // ( Qt::PointingHandCursor );
}

void Linker::stopLinking() // Static
{
     if( !m_selecComp ) return;
     m_selecComp->compSelected( nullptr );

     QGuiApplication::restoreOverrideCursor();
}

void Linker::compSelected( Component* comp )
{
    if( comp )  // One Component was selected to link
    {
        bool linked = m_linkedComp.contains( comp );

        if( linked )
        {
            comp->setLinkedTo( nullptr );
            comp->m_linkNumber = -1;
            m_linkedComp.removeAll( comp );
        }else{
            if( comp->setLinkedTo( this ) ) m_linkedComp.append( comp );
        }
        showLinked( true );
    }
    else       // End of linking Components
    {
        m_selecComp = nullptr;
        showLinked( false );
    }
}

void Linker::showLinked( bool show )
{
    if( show ){
        for( int i=0; i<m_linkedComp.size(); ++i ) // Set numbers for visualization
        {
            Component* comp = m_linkedComp.at( i );
            comp->m_linkNumber = i;
            comp->update();
        }
    }else{
        for( Component* comp : m_linkedComp )  // Clear numbers for visualization
            comp->m_linkNumber = -1;
    }
    Circuit::self()->update();
}
