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

#include "ui_memtable.h"

#ifndef MEMTABLE_H
#define MEMTABLE_H

class Component;

class MemTable : public QWidget, private Ui::MemTable
{
    Q_OBJECT
    
    public:
        MemTable( QWidget* parent=0, int dataSize=256, int wordBytes=1 );

        void updateTable(QVector<int>* data );
        void resizeTable( int dataSize );
        void setData(QVector<int>* data );
        void setValue( int address, int val );

    signals:
        void dataChanged( int address, int val );

    public slots:
        void on_table_itemChanged( QTableWidgetItem* item );
        void on_table_cellClicked( int row, int col ) { cellClicked( row, col ); }

    private:
        void cellClicked( int row, int col );
        QString valToHex( int val, int bytes );

        int m_dataSize;
        int m_wordBytes;
        int m_updtCount;

        bool m_blocked;

        QVector<int>* m_data;
};

#endif
