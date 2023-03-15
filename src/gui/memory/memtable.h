/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <QWidget>

#include "ui_memtable.h"

class Component;

class MemTable : public QWidget, private Ui::MemTable
{
    Q_OBJECT
    
    public:
        MemTable( QWidget* parent=0, int dataSize=256, int wordBytes=1 );

        void updateTable(QVector<int>* data );
        void setData(QVector<int>* data, int wordBytes=1 );
        void setValue( int address, int val );
        void setCellBytes( int bytes );
        void setAddrSelected( int addr ,bool jump );

    signals:
        void dataChanged( int address, int val );

    public slots:
        void on_table_itemChanged( QTableWidgetItem* item );
        void on_table_cellClicked( int row, int col ) { cellClicked( row, col ); }

    private:
        void resizeTable( int dataSize );
        void setCellValue( int address, int val );
        void cellClicked( int row, int col );
        QString valToHex( int val, int bytes );

        int m_updtCount;
        int m_dataSize;
        int m_wordBytes;
        int m_cellBytes;
        int m_byteRatio; // m_wordBytes/m_cellBytes

        bool m_blocked;

        QVector<int>* m_data;
};

#endif
