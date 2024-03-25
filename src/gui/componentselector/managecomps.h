/***************************************************************************
 *   Copyright (C) 2023 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MANAGECOMPS_H
#define MANAGECOMPS_H

#include <QDialog>

#include "ui_managecomps.h"

class QTreeWidgetItem;

class manCompDialog : public QDialog, private Ui::manCompDialog
{
    public:
        manCompDialog( QWidget* parent );

        void initialize();

    private slots:
        void slotItemChanged( QTableWidgetItem* item );

    private:
        void reject();
        void addItem( QTreeWidgetItem* item );
        void setItemVisible( QTableWidgetItem* item, bool visible );

        QHash<QTableWidgetItem*, QTreeWidgetItem*> m_qtwItem;
        QHash<QString, QString> m_shortCuts;

        bool m_initialized;
};
#endif
