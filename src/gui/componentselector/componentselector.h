/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPONENTSELECTOR_H
#define COMPONENTSELECTOR_H

#include <QDir>

#include "compplugindialog.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT ComponentSelector : public QTreeWidget
{
    Q_OBJECT
    
    public:
        ComponentSelector( QWidget* parent );
        ~ComponentSelector();

 static ComponentSelector* self() { return m_pSelf; }

        //void addLibraryItem( LibraryItem* libItem );
        //void removeLibItem( LibraryItem* libItem );

        void LoadCompSetAt( QDir compSetDir );
        void loadXml( const QString &setFile );

        QString getXmlFile( QString compName ) { return m_xmlFileList[ compName ]; }

        void search( QString filter );

    private slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        void slotContextMenu( const QPoint& );
        void slotManageComponents();

    private:
 static ComponentSelector* m_pSelf;

        void addItem( LibraryItem* libItem );
        void addItem( QString caption,
                      QTreeWidgetItem* catItem,
                      QString icon,
                      QString type );

        void LoadLibraryItems();
        //void LoadCompSet();
        //void istallPlugin( QString item );
        //void unistallPlugin( QString item );

        //void reLoadItems();

        QTreeWidgetItem* getCategory( QString _category, QString icon="" );

        QStringList m_compSetUnique;
        QStringList m_categories;
        //QString m_lastItemClicked;
        QHash<QString, QString> m_xmlFileList;

        CompPluginDialog m_pluginsdDialog;

        ItemLibrary m_itemLibrary;
};

#endif
