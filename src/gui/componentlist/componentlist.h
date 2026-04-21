/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include <QDropEvent>
#include <QDir>
#include <QTreeWidget>

#include "managecomps.h"
#include "itemlibrary.h"

class TreeItem;
class QDomNode;

class ComponentList : public QTreeWidget
{
    public:
        ComponentList( QWidget* parent );
        ~ComponentList();

 static ComponentList* self() { return m_pSelf; }

        void createList();
        void LoadCompSetAt( QDir compSetDir );

        QString getDataFile( QString compName ) { return m_dataFileList.value( compName ); }
        QString getFileDir( QString compName ) { return m_dirFileList.value( compName ); }

        void search( QString filter );

        void setShortcut( QString s, QString c ) { m_shortCuts[s] = c; }
        QString getComponent( QString shortcut ) { return m_shortCuts.value( shortcut ); }

        QStringList getxmlItems(){ return m_xmlItems; }

        void writeSettings();

        void setTheme( bool dark );

    public slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        void slotContextMenu( const QPoint& );
        void slotManageComponents();

    protected:
        void mousePressEvent( QMouseEvent* event ) override;
        void dropEvent( QDropEvent* event ) override;

    private:
        void loadXml( QString xmlFile );
        void addItem( QString caption, TreeItem* catItem, QString icon, QString type );
        void addItem( QString caption, TreeItem* catItem, QPixmap &icon, QString type );
        void addLibraryItem( LibraryItem* item );

        void LoadLibraryItems();
        void loadTest( QString userDir );
        void readConfig();
        void readNodCfg( QDomNode* node, TreeItem* parent );

        TreeItem* getCategory( QString category );
        TreeItem* addCategory( QString nameTr, QString name, QString parent, QString icon );

        TreeItem* m_clickedItem;

        QString getIcon( QString folder, QString name );

        bool m_customComp;
        bool m_oldConfig;
        QString m_listFile;

        QString m_searchFilter;

        QMap<QString, TreeItem*> m_components;
        QMap<QString, TreeItem*> m_categories;

        QList<TreeItem*> m_categoryList;

        QMap<QString, QString> m_dataFileList;
        QMap<QString, QString> m_dirFileList;
        QMap<QString, QString> m_shortCuts;

        QStringList m_xmlItems;

        QDir m_compSetDir;

        manCompDialog m_mcDialog;

        ItemLibrary m_itemLibrary;

 static ComponentList* m_pSelf;
};
