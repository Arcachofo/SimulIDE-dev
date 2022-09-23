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
    Q_DECLARE_TR_FUNCTIONS( ItemLibrary )
    
    public:
        ItemLibrary();
        ~ItemLibrary();

 static ItemLibrary* self() { return m_pSelf; }

        const QList<LibraryItem*> items() const;

        LibraryItem*  libraryItem( const QString type ) const;

        LibraryItem*  itemByName( const QString name ) const;

        void addItem( LibraryItem* item );
        
        void loadItems();
        
        //void loadPlugins();

    
    protected:
 static ItemLibrary* m_pSelf;

        QList<LibraryItem*> m_items;
        //QStringList m_plugins;
        
        friend ItemLibrary*  itemLibrary();
};


class MAINMODULE_EXPORT LibraryItem
{
    public:
        LibraryItem( const QString &name, const QString &category, const QString &iconName,
                     const QString type, createItemPtr createItem );
        
        ~LibraryItem();

        QString name()     const { return m_name; }
        QString category() const { return m_category; }
        QString iconfile() const { return m_iconfile; }
        QString type()     const { return m_type; }

        createItemPtr createItemFnPtr() const { return createItem; }

    private:
        QString m_name;
        QString m_category;
        QString m_iconfile;
        QString m_type;

        createItemPtr createItem;
};

#endif
