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
        : QWidget( parent )
{
    setupUi(this);

    m_wordBytes = wordBytes;
    m_updtCount = 0;
    m_data = NULL;

    resizeTable( dataSize );
}

void MemTable::updateTable( QVector<int>* data )
{
    if( ++m_updtCount >= 10 ) m_updtCount = 0;
    else                      return;
    setData( data );
}

void MemTable::setValue( int address, int val )
{
    int row = address/16;
    int colRam = address%16;
    int colAscii = colRam +17;
    m_blocked = true;
    table->item( row, colRam )->setData( 0, valToHex( val, m_wordBytes ) );
    QString valS = QChar( val&0x00FF );
    for( int i=1; i<m_wordBytes; ++i )
    {
        val >>= 8;
        valS += " "+QString( QChar( val&0x00FF ) );
    }
    table->item( row, colAscii )->setData( 0, valS );

    m_blocked = false;
}

void MemTable::setData( QVector<int>* data )
{
    m_data = data;
    if( data->size() != m_dataSize ) resizeTable( data->size() );

    for( int i=0; i<data->size(); ++i ) setValue( i, data->at(i) );
}

void MemTable::resizeTable( int dataSize )
{
    m_blocked = true;

    m_dataSize = dataSize;
    int addrBytes = ceil( ceil(log2(dataSize))/8 );

    int rows = dataSize/16;
    if( dataSize%16) rows++;

    table->clear();

    table->setHorizontalHeaderLabels( QStringList()
      <<"0x0"<<"0x1"<<"0x2"<<"0x3"<<"0x4"<<"0x5"<<"0x6"<<"0x7"
      <<"0x8"<<"0x9"<<"0xA"<<"0xB"<<"0xC"<<"0xD"<<"0xE"<<"0xF"
      << " "
      <<"0"<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"
      <<"8"<<"9"<<"A"<<"B"<<"C"<<"D"<<"E"<<"F");

    table->setColumnWidth( 16, 5 );
    table->setRowCount( rows );

    int scale = MainWindow::self()->fontScale();
    QFont font;
    font.setPixelSize( 13*scale );
    font.setFamily("Ubuntu Mono");

    QTableWidgetItem* it;

    for( int row=0; row<rows; ++row )
    {
        it = new QTableWidgetItem(0);
        it->setFlags( Qt::ItemIsEnabled );
        it->setFont( font );
        it->setText( valToHex( row*16, addrBytes ));
        table->setVerticalHeaderItem( row, it );

        for( int col=0; col<33; ++col )
        {
            it = new QTableWidgetItem(0);
            if( col == 16 ) it->setFlags( 0 );
            else
            {
                it->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
                it->setFont( font );
            }
            table->setItem( row, col, it );
        }
        table->setRowHeight( row, 20*scale );
    }
    for( int col=0; col<16; ++col )
    {
        table->setColumnWidth( col, (2+m_wordBytes)*15*scale+2 );
        table->setColumnWidth( col+17, 15*m_wordBytes*scale );
    }
    font.setBold(true);
    table->horizontalHeader()->setFont( font );
    table->verticalHeader()->setFont( font );
    m_blocked = false;
}

void MemTable::on_table_itemChanged( QTableWidgetItem* item )
{
    if( m_blocked ) return;
    m_blocked = true;
    int val=0;
    bool ok;

    int col = item->column();
    if( col > 16 )
    {
        col -= 17;
        ok = item->text().size() == m_wordBytes;
        if( ok )
        {
            for( int i=0; i<m_wordBytes; ++i )
            {
                QChar cv = item->text().at(i);
                val += cv.cell() << (8*i);
    }   }   }
    else val = item->text().toInt( &ok, 0 );

    int address = item->row()*16+col;
    if( ok )
    {
        if( m_data) m_data->replace( address, val );
        setValue( address, val );
        emit dataChanged( address, val );
    }
    else if( m_data) setValue( address, m_data->at( address ) );

    m_blocked = false;
}

void MemTable::cellClicked( int row, int col )
{
    if( col == 16 ) return;

    if( col > 16 ) col -= 17;
    table->clearSelection();
    table->item( row, col )->setSelected( true );
    table->item( row, col+17 )->setSelected( true );
}

QString MemTable::valToHex( int val, int bytes )
{
    QString sval = QString::number( val, 16 ).toUpper();
    sval = sval.right( bytes*2 );
    while( sval.length() < bytes*2) sval.prepend( "0" );
    sval.prepend(" 0x");
    sval.append(" ");

    return sval;
}
