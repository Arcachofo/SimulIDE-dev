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

#include "ramtable.h"
#include "mcuinterface.h"
#include "basedebugger.h"
#include "mainwindow.h"
#include "utils.h"

RamTable::RamTable( QWidget* parent, McuInterface* processor )
        : QWidget( parent )
        , m_status( 1, 8 )
        , m_pc( 1, 2 )
{
    setupUi(this);

    m_processor = processor;
    m_debugger  = NULL;
    m_numRegs = 60;
    m_loadingVars = false;

    int scale = MainWindow::self()->fontScale();
    int row_heigh = 23*scale;
    int numberColor = 0x202090;

    QTableWidgetItem* it;
    QFont font;
    font.setFamily("Ubuntu Mono");
    font.setBold(true);
    font.setPixelSize( 14*scale );
    m_status.setFont(font);
    m_pc.setFont(font);

    m_status.setVerticalHeaderLabels( QStringList()<<" STATUS " );
    m_status.horizontalHeader()->hide();
    m_status.setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_status.setRowHeight( 0, row_heigh );
    m_status.setFixedHeight( row_heigh );
    font.setPixelSize( 12*scale );
    for( int i=0; i<8; i++ )
    {
        m_status.setColumnWidth( i, 22*scale );
        it = new QTableWidgetItem(0);
        it->setFlags( Qt::ItemIsEnabled );
        it->setFont( font );
        m_status.setItem( 0, i, it );
    }
    m_status.setMinimumWidth( (62+8*22)*scale );
    m_status.setMaximumWidth( (62+8*22)*scale );

    m_pc.setVerticalHeaderLabels( QStringList()<<" PC "  );
    m_pc.horizontalHeader()->hide();
    m_pc.setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_pc.setRowHeight( 0, row_heigh );
    m_pc.setFixedHeight( row_heigh  );

    font.setPixelSize( 13*scale );
    it = new QTableWidgetItem(0);
    it->setFlags( Qt::ItemIsEnabled );
    it->setFont( font );
    it->setTextColor( QColor( numberColor ) );
    m_pc.setItem( 0, 0, it );
    m_pc.setColumnWidth(0, 45*scale);

    it = new QTableWidgetItem(0);
    it->setFlags( Qt::ItemIsEnabled );
    it->setFont( font );
    it->setTextColor( QColor( 0x3030B8 ) );
    m_pc.setItem( 0, 1, it );
    m_pc.setColumnWidth(1, 60*scale);
    m_pc.setMinimumWidth( 130*scale );
    m_pc.setMaximumWidth( 130*scale );

    table->verticalHeader()->setSectionsMovable( true );
    table->setColumnWidth( 0, 50*scale );
    table->setColumnWidth( 1, 55*scale );
    table->setColumnWidth( 2, 40*scale );
    table->setColumnWidth( 3, 40*scale );
    table->setColumnWidth( 4, 75*scale );

    for( int row=0; row<m_numRegs; row++ )
    {
        it = new QTableWidgetItem(0);
        it->setFont( font );
        it->setText( "---" );
        table->setVerticalHeaderItem( row, it );
        for( int col=0; col<5; col++ )
        {
            QTableWidgetItem *it = new QTableWidgetItem(0);
            if( col != 1 ){
                it->setFlags( Qt::ItemIsEnabled );
                it->setText("---");
                if( col == 2 ) it->setTextColor( QColor( 0x904020 ) );
                else           it->setTextColor( QColor( numberColor ) );
            }
            it->setFont( font );
            table->setItem( row, col, it );
        }
        table->setRowHeight(row, row_heigh);
    }
    table->setHorizontalHeaderLabels( QStringList()<<tr("Addr")<<tr("Name")<<tr("Type")<<tr("Dec")<<tr("Value")  );

    setContextMenuPolicy( Qt::CustomContextMenu );

    registers->setFont( font );
    //registers->setFixedWidth( 80*scale );
    registers->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_registerModel = new QStandardItemModel(this);
    registers->setModel( m_registerModel );

    connect( registers, SIGNAL(doubleClicked(QModelIndex)),
             this,      SLOT(RegDoubleClick(QModelIndex)));

    connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT  (slotContextMenu(const QPoint&)), Qt::UniqueConnection);

    connect( table, SIGNAL(itemChanged(QTableWidgetItem*)  ),
             this, SLOT(addToWatch(QTableWidgetItem*)), Qt::UniqueConnection );
}

void RamTable::RegDoubleClick(const QModelIndex& index)
{
    m_currentRow = table->currentRow();
    if( m_currentRow < 0 ) return;

    setItemValue( 1, m_registerModel->item(index.row())->text() );
}

void RamTable::setStatusBits( QStringList statusBits )
{
    for( int i=7; i>=0; --i )
        m_status.item( 0, i )->setText( statusBits.takeFirst() );
}

void RamTable::slotContextMenu( const QPoint& point )
{
    QMenu menu;
    if( m_debugger )
    {
        QAction *loadVars = menu.addAction( QIcon(":/open.png"),tr("Load Variables") );
        connect( loadVars, SIGNAL(triggered()), this, SLOT(loadVariables()), Qt::UniqueConnection );
    }

    QAction *clearSelected = menu.addAction( QIcon(":/remove.png"),tr("Clear Selected") );
    connect( clearSelected, SIGNAL(triggered()), this, SLOT(clearSelected()), Qt::UniqueConnection );

    QAction *clearTable = menu.addAction( QIcon(":/remove.png"),tr("Clear Table") );
    connect( clearTable, SIGNAL(triggered()), this, SLOT(clearTable()), Qt::UniqueConnection );

    menu.addSeparator();

    QAction *loadVarSet = menu.addAction( QIcon(":/open.png"),tr("Load VarSet") );
    connect( loadVarSet, SIGNAL(triggered()), this, SLOT(loadVarSet()), Qt::UniqueConnection );

    QAction *saveVarSet = menu.addAction( QIcon(":/save.png"),tr("Save VarSet") );
    connect( saveVarSet, SIGNAL(triggered()), this, SLOT(saveVarSet()), Qt::UniqueConnection );

    menu.exec( mapToGlobal(point) );
}

void RamTable::clearSelected()
{
    for( QTableWidgetItem* item : table->selectedItems() ) item->setData( 0, "");
}

void RamTable::clearTable()
{
    for( QTableWidgetItem* item : table->findItems( "*", Qt::MatchWildcard)  )
    { if( item ) item->setData( 0, "");}
    table->setCurrentCell( 0, 0 );
}

void RamTable::loadVariables()
{
    if( !m_debugger ) return;

    m_loadingVars = true;
    QStringList variables = m_debugger->getVarList();

    loadVarSet( variables );
}

void RamTable::loadVarSet()
{
    const QString dir = m_processor->getFileName();

    QString fileName = QFileDialog::getOpenFileName( this, tr("Load VarSet"), dir, tr("VarSets (*.vst);;All files (*.*)"));

    if( !fileName.isEmpty() )
    {
        QStringList varSet = fileToStringList( fileName, "RamTable::loadVarSet" );
        if( !varSet.isEmpty() ) loadVarSet( varSet );
}   }

void RamTable::loadVarSet( QStringList varSet )
{
    if( !m_loadingVars )
    {
        m_loadingVars = true;
        table->setCurrentCell( 0, 1 ); // Loading varset
    }
    int row = table->currentRow()-1;
    for( QString var : varSet )
    {
        row++;
        if( row >= m_numRegs ) break;
        table->item( row, 1 )->setText( var );
    }
    table->setCurrentCell( 0, 1 );

    m_loadingVars = false;
}

QStringList RamTable::getVarSet()
{
    bool empty = true;
    QStringList varset;
    for( int row=m_numRegs-1; row>=0; --row )
    {
        QString name = table->item( row, 1 )->text();
        if( !name.isEmpty() ) empty = false;
        if( empty ) continue;
        varset.prepend( name );
    }
    return varset;
}

void RamTable::saveVarSet()
{
    const QString dir = m_processor->getFileName();

    QString fileName = QFileDialog::getSaveFileName( this, tr("Save VarSet"), dir,
                                                 tr("VarSets (*.vst);;All files (*.*)"));
    if( !fileName.isEmpty() )
    {
        if( !fileName.endsWith(".vst") ) fileName.append(".vst");

        QFile file( fileName );

        if( !file.open(QFile::WriteOnly | QFile::Text) )
        {
              QMessageBox::warning(0l, "RamTable::saveVarSet",
              tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
              return;
        }
        file.flush();

        QTextStream out(&file);
        out.setCodec( "UTF-8" );
        QApplication::setOverrideCursor(Qt::WaitCursor);

        for( int row=0; row<m_numRegs; row++ )
        {
            QString name = table->item( row, 1 )->text();
            out << name << "\n";
        }
        file.close();
        QApplication::restoreOverrideCursor();
}   }

void RamTable::updateValues()
{
    if( !m_processor ) return;

    for( int _row: watchList.keys() )
    {
        m_currentRow = _row;
        QString name = watchList[_row];

        bool ok;
        int addr = name.toInt(&ok, 10);
        if( !ok ) addr = name.toInt(&ok, 16);
        if( !ok ) m_processor->updateRamValue( name );  // Var or Reg name
        else                                            // Address
        {
            uint8_t value = m_processor->getRamValue( addr );

            table->item( _row, 2 )->setText("uint8");
            table->item( _row, 3 )->setData( 0, value );
            table->item( _row, 4 )->setData( 0, decToBase(value, 2, 8) );
}   }   }

void RamTable::addToWatch( QTableWidgetItem* it )
{
    if( table->column(it) != 1 ) return;
    int _row = table->row(it);
    table->setCurrentCell( _row, 1 );

    QString name = it->text().remove(" ").remove("\t").remove("*");//.toLower();

    if( name.isEmpty() )
    {
        watchList.remove(_row);
        table->verticalHeaderItem( _row )->setText("---");

        table->item( _row, 0 )->setText("---");
        table->item( _row, 2 )->setText("---");
        table->item( _row, 3 )->setText("---");
        table->item( _row, 4 )->setText("---");
    }else{
        int addr = m_processor->getRegAddress( name );
        if( addr == 65535 )
        {
            bool ok;
            addr = name.toInt(&ok, 10);
            if( !ok ) addr = name.toInt(&ok, 16);
            if( !ok ) addr = -1;
        }
        if( addr >= 0 )
        {
            watchList[_row] = name;
            table->verticalHeaderItem( _row )->setData( 0, addr );
            table->item( _row, 0 )->setText( "0x"+decToBase(addr, 16, 4).remove(0,1) );
        }
        if( !m_debugger ) return;
        QString varType = m_debugger->getVarType( name );

        if( !m_loadingVars && varType.contains( "array" ) )
        {
            int size = varType.replace( "array", "" ).toInt();

            QStringList variables = m_debugger->getVarList();

            int indx = variables.indexOf( name );
            int listEnd = variables.size()-1;
            for( int i=1; i<size ; i++ )
            {
                int index = indx+i;
                if( index > listEnd ) break;

                QString varName = variables.at( index );
                if( varName.contains( name ) ) table->item( _row+i, 1 )->setText( varName );
}   }   }   }

void RamTable::setRegisters( QStringList regs )
{
    regs.sort();
    for( QString reg : regs ) m_registerModel->appendRow( new QStandardItem(reg) );
}

uint16_t RamTable::getCurrentAddr()
{
    QString text = table->item( table->currentRow(), 0 )->text();
    if( text == "---" ) return -1;
    return  text.toUInt(NULL,16);
}

//#include "moc_ramtable.cpp"
