/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ITEMLIBRARY_H
#define ITEMLIBRARY_H

#include <QCoreApplication>

#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT ItemLibrary
{
    //Q_DECLARE_TR_FUNCTIONS( ItemLibrary )
    
    public:
        ItemLibrary();
        ~ItemLibrary();

 static ItemLibrary* self() { return m_pSelf; }

        QList<LibraryItem*> items() { return m_items; }

        LibraryItem* itemByName( const QString name );

        void addItem( LibraryItem* item );
        
        void loadItems();

    protected:
 static ItemLibrary* m_pSelf;

        QList<LibraryItem*> m_items;
};


class MAINMODULE_EXPORT LibraryItem
{
    public:
        LibraryItem( QString name, QString category, QString iconName,
                     QString type, createItemPtr createItem );
        
        ~LibraryItem();

        QString name()     { return m_name; }
        QString category() { return m_category; }
        QString iconfile() { return m_iconfile; }
        QString type()     { return m_type; }

        createItemPtr createItemFnPtr() { return createItem; }

    private:
        QString m_name;
        QString m_category;
        QString m_iconfile;
        QString m_type;

        createItemPtr createItem;
};

#endif
