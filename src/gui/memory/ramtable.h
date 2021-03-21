/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include <QWidget>

#include "ui_ramtable.h"

#ifndef RAMTABLE_H
#define RAMTABLE_H

class BaseProcessor;
class BaseDebugger;
class QStandardItemModel;

class RamTable : public QWidget, private Ui::RamTable
{
    Q_OBJECT

    public:
        RamTable( QWidget* parent=0, BaseProcessor* processor=0 );

        void updateValues();

        void setItemValue( int col, QString value );
        void setItemValue( int col, int32_t value );
        void setItemValue( int col, float value );

        void setStatusBits( QStringList statusBits );

        void setRegisters();
        void setDebugger( BaseDebugger*  deb );
        void remDebugger( BaseDebugger*  deb );

        void loadVarSet( QStringList varSet );
        QStringList getVarSet();

        QTableWidget m_status;
        QTableWidget m_pc;

    public slots:
        void RegDoubleClick( const QModelIndex& index );
        void clearSelected();
        void clearTable();
        void loadVarSet();
        void saveVarSet();
        void loadVariables();

    private slots:
        void addToWatch( QTableWidgetItem* );
        void slotContextMenu( const QPoint& );

    private:
        BaseProcessor* m_processor;
        BaseDebugger*  m_debugger;

        QStandardItemModel* m_registerModel;

        QHash<int, QString> watchList;

        bool m_loadingVars;

        int m_numRegs;
        int m_currentRow;
};
#endif // RAMTABLE_H
