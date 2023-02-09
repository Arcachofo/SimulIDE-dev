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

        void LoadCompSetAt( QDir compSetDir );

        QString getXmlFile( QString compName ) { return m_xmlFileList[ compName ]; }

        void search( QString filter );

        QTreeWidgetItem* getCategory( QString category );
        QStringList getCategories(){ return m_categories.keys(); }

    private slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        //void slotContextMenu( const QPoint& );
        void slotManageComponents();

    private:
 static ComponentSelector* m_pSelf;

        void loadXml( const QString &setFile );

        void addItem( QString caption, QTreeWidgetItem* catItem, QString icon, QString type );

        void LoadLibraryItems();

        QTreeWidgetItem* addCategory( QString nameTr, QString name, QString parent, QString icon );

        QStringList m_components;
        QHash<QString, QTreeWidgetItem*> m_categories;
        QHash<QString, QString> m_catTr;

        QHash<QString, QString> m_xmlFileList;

        CompPluginDialog m_pluginsdDialog;

        ItemLibrary m_itemLibrary;
};

#endif
