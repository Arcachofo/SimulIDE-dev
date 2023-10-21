/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINKABLE_H
#define LINKABLE_H

#include <QSet>
#include <QCursor>

class Component;

class Linkable
{
    public:
        Linkable();
        ~Linkable();

        QString getLinks();
        void setLinks( QString links );

        Component* getLinkedComp( int i ) { return m_linkedComp.value( i ); }

        bool hasLinks() { return !m_linkedStr.isEmpty(); }

        void startLinking();
 static void stopLinking();

 static Linkable* m_selecComp;

        virtual void createLinks( QSet<Component*>* );
        virtual void compSelected( Component* comp );

    protected:
        QList<Component*> m_linkedComp;

        QString m_linkedStr;

        QCursor m_linkCursor;
};

#endif
