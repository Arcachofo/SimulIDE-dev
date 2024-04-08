/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPONENTSELECTOR_H
#define COMPONENTSELECTOR_H

#include <QDir>
#include <QTreeWidget>

#include "managecomps.h"
#include "itemlibrary.h"

class ComponentSelector : public QTreeWidget
{
    public:
        ComponentSelector( QWidget* parent );
        ~ComponentSelector();

 static ComponentSelector* self() { return m_pSelf; }

        void LoadCompSetAt( QDir compSetDir );

        QString getDataFile( QString compName ) { return m_dataFileList.value( compName ); }
        QString getFileDir( QString compName ) { return m_dirFileList.value( compName ); }

        void search( QString filter );

        void hideFromList( QTreeWidgetItem* item, bool hide );

        QTreeWidgetItem* getCategory( QString category );
        QStringList getCategories(){ return m_categories.keys(); }

        void setShortCuts( QHash<QString, QString> s ) { m_shortCuts = s; }
        QHash<QString, QString> getShortCuts() { return m_shortCuts; }

        void loadXml( QString setFile, bool convert=false );

    private slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        void slotContextMenu( const QPoint& );
        void slotManageComponents();

    private:
 static ComponentSelector* m_pSelf;

        void convertItem( QString folder, QString itemFile, QString name, QString category, QString icon, QString type );
        QString convertMcuFile( QString file );

        void addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type );
        void addItem( QString caption, QTreeWidgetItem* catItem, QIcon &icon, QString type );

        void LoadLibraryItems();

        void loadComps( QDir compSetDir );

        bool m_customComp;

        QTreeWidgetItem* addCategory( QString nameTr, QString name, QString parent, QString icon );

        QString getIcon( QString folder, QString name );

        QStringList m_components;
        QHash<QString, QTreeWidgetItem*> m_categories;
        QHash<QString, QString> m_catTr;

        QHash<QString, QString> m_dataFileList;
        QHash<QString, QString> m_dirFileList;
        QHash<QString, QString> m_shortCuts;

        QDir m_compSetDir;

        manCompDialog m_mcDialog;

        ItemLibrary m_itemLibrary;
};

#endif
