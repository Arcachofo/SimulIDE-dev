/***************************************************************************
 *   Copyright (C) 2023 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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
