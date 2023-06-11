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

class MAINMODULE_EXPORT ComponentSelector : public QTreeWidget
{
    public:
        ComponentSelector( QWidget* parent );
        ~ComponentSelector();

 static ComponentSelector* self() { return m_pSelf; }

        void LoadCompSetAt( QDir compSetDir );

        QString getXmlFile( QString compName ) { return m_xmlFileList.value( compName ); }
        QString getFileDir( QString compName ) { return m_dirFileList.value( compName ); }

        void search( QString filter );

        void hideFromList( QTreeWidgetItem* item, bool hide );

        QTreeWidgetItem* getCategory( QString category );
        QStringList getCategories(){ return m_categories.keys(); }

        void setShortCuts( QHash<QString, QString> s ) { m_shortCuts = s; }
        QHash<QString, QString> getShortCuts() { return m_shortCuts; }

    private slots:
        void slotItemClicked( QTreeWidgetItem* item, int );
        void slotContextMenu( const QPoint& );
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
        QHash<QString, QString> m_dirFileList;
        QHash<QString, QString> m_shortCuts;

        manCompDialog m_mcDialog;

        ItemLibrary m_itemLibrary;
};

#endif
