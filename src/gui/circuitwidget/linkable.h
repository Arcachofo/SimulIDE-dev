/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LINKABLE_H
#define LINKABLE_H

#include <QList>

class Component;

class MAINMODULE_EXPORT Linkable
{
    public:
        Linkable();
        ~Linkable();

        QString getLinks();
        void setLinks( QString links );

        bool hasLinks() { return !m_linkedStr.isEmpty(); }

        void startLinking();

        virtual void createLinks( QList<Component*>* );
        virtual void compSelected( Component* comp );

    protected:
        QList<Component*> m_linkedComp;

        QString m_linkedStr;
};

#endif
