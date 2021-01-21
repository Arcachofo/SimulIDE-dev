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

#include "memtable.h"
#include "mainwindow.h"

MemTable::MemTable( QWidget* parent, int dataSize, int wordBytes )
        : QDialog( parent )
{
    setupUi(this);

    m_wordBytes = wordBytes;
    m_updtCount = 0;

    table->setHorizontalHeaderLabels( QStringList()
      <<"0x0"<<"0x1"<<"0x2"<<"0x3"<<"0x4"<<"0x5"<<"0x6"<<"0x7"
      <<"0x8"<<"0x9"<<"0xA"<<"0xB"<<"0xC"<<"0xD"<<"0xE"<<"0xF");

    resizeTable( dataSize );
}

void MemTable::updateTable( QVector<int> data )
{
    if( ++m_updtCount == 10 ) m_updtCount = 0;
    else                      return;

    if( data.size() != m_dataSize ) resizeTable( data.size() );

    int row = 0;
    int col = 0;

    for( int val : data )
    {
        QString sval = QString::number( val, 16 ).toUpper();
        while( sval.length() < m_wordBytes*2) sval.prepend( "0" );
        sval.prepend("0x");
        table->item( row, col )->setData( 0, sval );

        if( ++col == 16 ) { row++; col = 0; }
    }
}

void MemTable::resizeTable( int dataSize )
{
    m_dataSize = dataSize;

    int rows = dataSize/16;
    if( dataSize%16) rows++;

    table->setRowCount( rows );

    int scale = MainWindow::self()->fontScale();
    QFont font;
    font.setBold(true);
    font.setPixelSize( 12*scale );
    font.setFamily("Monospace");

    QTableWidgetItem* it;

    int row_heigh = 20*scale;
    int col_width = (1+m_wordBytes)*20*scale;

    for( int row=0; row<rows; ++row )
    {
        QString addr = QString::number( row*16, 16 ).toUpper()+" ";
        while( addr.length() < 4) addr.prepend( "0" );
        addr.prepend("0x");

        it = new QTableWidgetItem(0);
        it->setFlags( Qt::ItemIsEnabled );
        it->setFont( font );

        table->setVerticalHeaderItem( row, it );
        table->verticalHeaderItem( row )->setText( addr );
        table->verticalHeaderItem( row )->setFont( font );

        for( int col=0; col<16; ++col )
        {
            QTableWidgetItem *it = new QTableWidgetItem(0);
            it->setFlags( Qt::ItemIsEnabled );
            it->setFont( font );
            table->setItem( row, col, it );
        }
        table->setRowHeight(row, row_heigh);
    }
    for( int col=0; col<16; ++col )
    {
        table->setColumnWidth( col, col_width );
        table->horizontalHeaderItem( col )->setFont( font );
    }
}
