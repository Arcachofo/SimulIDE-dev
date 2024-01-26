/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINKER_H
#define LINKER_H

#include <QSet>
#include <QCursor>

class Component;

class Linker
{
    public:
        Linker();
        ~Linker();

        QString getLinks();
        void setLinks( QString links );

        Component* getLinkedComp( int i ) { return m_linkedComp.value( i ); }

        bool hasLinks() { return !m_linkedStr.isEmpty(); }

        void startLinking();
 static void stopLinking();

 static Linker* m_selecComp;

        virtual void createLinks( QList<Component*>* );
        virtual void compSelected( Component* comp );

        void showLinked( bool show );

    protected:
        QList<Component*> m_linkedComp;

        QString m_linkedStr;

        QCursor m_linkCursor;
};

#endif
