/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
 
#ifndef COMPPLUGINDIALOG_H
#define COMPPLUGINDIALOG_H

#include <QDialog>
#include <QtWidgets/QListWidget>
#include <QTreeWidgetItem>

namespace Ui {
class ComponentPlugins;
}

class CompPluginDialog : public QDialog
{
    Q_OBJECT

    public:
        CompPluginDialog(QWidget *parent);
        ~CompPluginDialog();

        void setPluginList();

    private slots:
        void slotItemChanged( QListWidgetItem* item );
        
    private:
        Ui::ComponentPlugins* ui;

        void reject();

        void addItem( QTreeWidgetItem* item );

        void setItemVisible(QListWidgetItem* listItem, bool visible );


        QHash<QListWidgetItem*, QTreeWidgetItem*>  m_qtwItem;

        QListWidget* m_compList;

        bool m_initialized;
};
#endif
