/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QTreeWidget>

enum treItemType_t{
    type_NONE=0,
    component,
    categ_MAIN,
    categ_CHILD,
};

class TreeItem : public QTreeWidgetItem
{
    public:
        TreeItem( TreeItem* parent, QString name, QString nameTr, QString compType, treItemType_t itemType, const QIcon &icon, bool custom );
        ~TreeItem();

        QString name()     { return m_name; }
        QString nameTr()   { return m_nameTr; }
        QString compType() { return m_compType; }

        TreeItem* parentItem() { return m_parent; }

        QString shortcut() { return m_shortcut; }
        void setShortCut( QString s ) { m_shortcut = s; }

        bool isItemExpanded() { return m_expanded; }
        void setItemExpanded( bool e );

        bool isItemHidden()   { return m_hidden; }
        void setItemHidden( bool h );

        void setItemType( treItemType_t itemType );

        QString toString( QString indent="" );

    private:

        QString m_name;
        QString m_nameTr;
        QString m_compType;
        QString m_shortcut;

        bool m_isCustom;
        bool m_expanded;
        bool m_hidden;

        treItemType_t m_itemType;

        TreeItem* m_parent;
};

#endif
