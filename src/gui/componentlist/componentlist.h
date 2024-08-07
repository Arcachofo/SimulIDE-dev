/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPONENTLIST_H
#define COMPONENTLIST_H

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

        TreeItem* getCategory( QString category );
        QStringList getCategories(){ return m_categories.keys(); }

        void setShortcut( QString s, QString c ) { m_shortCuts[s] = c; }
        QString getComponent( QString shortcut ) { return m_shortCuts.value( shortcut ); }

        void loadXml( QString xmlFile );

        void writeSettings();

    public slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        void slotContextMenu( const QPoint& );
        void slotManageComponents();

    protected:
        void mousePressEvent( QMouseEvent* event ) override;
        void dropEvent( QDropEvent* event ) override;

    private:
 static ComponentList* m_pSelf;

        void addItem( QString caption, TreeItem* catItem, QString icon, QString type );
        void addItem( QString caption, TreeItem* catItem, QIcon &icon, QString type );

        void LoadLibraryItems();
        void readConfig();
        void readNodCfg( QDomNode* node, TreeItem* parent );

        TreeItem* addCategory( QString nameTr, QString name, QString parent, QString icon );

        QString getIcon( QString folder, QString name );

        bool m_customComp;
        bool m_oldConfig;
        bool m_restoreList;
        QString m_listFile;

        QString m_searchFilter;

        QHash<QString, TreeItem*> m_components;
        QHash<QString, TreeItem*> m_categories;
        QHash<QString, QString>   m_catNames;


        QHash<QString, QString> m_dataFileList;
        QHash<QString, QString> m_dirFileList;
        QHash<QString, QString> m_shortCuts;

        QDir m_compSetDir;

        manCompDialog m_mcDialog;

        ItemLibrary m_itemLibrary;
};
#endif
