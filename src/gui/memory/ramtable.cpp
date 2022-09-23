/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QStandardItemModel>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

#include <math.h>

#include "ramtable.h"
#include "e_mcu.h"
#include "cpubase.h"
#include "circuit.h"
#include "basedebugger.h"
#include "mainwindow.h"
#include "utils.h"

RamTable::RamTable( QWidget* parent, eMcu* processor ,bool cpuMonitor )
        : QWidget( parent )
{
    setupUi(this);

    m_processor = processor;
    m_cpuMonitor = cpuMonitor;
    m_debugger  = NULL;
    m_numRegs   = 60;
    m_loadingVars = false;

    float scale = MainWindow::self()->fontScale();
    int row_heigh = round( 22*scale );
    int font_size = round(14*scale);
    int numberColor = 0x202090;

    QTableWidgetItem* it;
    QFont font;
    font.setFamily("Ubuntu Mono");
    font.setBold( true );
    font.setPixelSize( font_size );

    table->verticalHeader()->setSectionsMovable( true );
    if( m_cpuMonitor )
    {
        table->setColumnWidth( 1, round(55*scale) );
        table->setColumnWidth( 2, round(60*scale) );
        table->setColumnWidth( 3, round(60*scale) );
        table->setColumnWidth( 4, round(90*scale) );
    }else{
        table->setColumnWidth( 0, round(50*scale) );
        table->setColumnWidth( 1, round(55*scale) );
        table->setColumnWidth( 2, round(40*scale) );
        table->setColumnWidth( 3, round(40*scale) );
        table->setColumnWidth( 4, round(75*scale) );
    }
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

    if( m_cpuMonitor )
    {
        table->verticalHeader()->setVisible( false );
        table->setColumnHidden( 0, true );
    }
    setContextMenuPolicy( Qt::CustomContextMenu );

    registers->setFont( font );
    //registers->setFixedWidth( 80*scale );
    registers->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_registerModel = new QStandardItemModel( this );
    registers->setModel( m_registerModel );

    variables->setFont( font );
    //registers->setFixedWidth( 80*scale );
    variables->setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_variableModel = new QStandardItemModel( this );
    variables->setModel( m_variableModel );

    splitter_2->setSizes( {100,30} );

    /*connect( registers, SIGNAL(doubleClicked(QModelIndex)),
             this,      SLOT(RegDoubleClick(QModelIndex)));*/

    connect( variables, SIGNAL(activated(QModelIndex)),
             this,      SLOT(VarDoubleClick(QModelIndex)));

    connect( registers, SIGNAL(activated(QModelIndex)),
             this,      SLOT(RegDoubleClick(QModelIndex)));

    connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT  (slotContextMenu(const QPoint&)), Qt::UniqueConnection);

    connect( table, SIGNAL(itemChanged(QTableWidgetItem*)  ),
             this, SLOT(addToWatch(QTableWidgetItem*)), Qt::UniqueConnection );
}

void RamTable::RegDoubleClick( const QModelIndex& index )
{
    m_currentRow = table->currentRow();
    if( m_currentRow < 0 ) return;

    setItemValue( 1, m_registerModel->item(index.row())->text() );
    table->setCurrentCell( m_currentRow+1, 1 );
}

void RamTable::VarDoubleClick( const QModelIndex& index )
{
    m_currentRow = table->currentRow();
    if( m_currentRow < 0 ) return;

    setItemValue( 1, m_variableModel->item(index.row())->text() );
    table->setCurrentCell( m_currentRow+1, 1 );
}

void RamTable::slotContextMenu( const QPoint& point )
{
    QMenu menu;

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

void RamTable::loadVarSet()
{
    const QString dir = Circuit::self()->getFilePath();

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
    const QString dir = Circuit::self()->getFilePath();

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



void RamTable::setRegisters( QStringList regs )
{
    regs.sort();
    m_regNames = regs;
    for( QString reg : regs ) m_registerModel->appendRow( new QStandardItem(reg) );
}

void RamTable::addRegister( QString name, QString type )
{
    m_regNames.append( name );
    m_typeTable[ name ] = type;
    m_registerModel->appendRow( new QStandardItem(name) );
}

void RamTable::setVariables( QStringList vars )
{
    vars.sort();
    m_varNames = vars;
    m_variableModel->clear();
    for( QString var : vars ) m_variableModel->appendRow( new QStandardItem(var) );
}

void RamTable::addVariable( QString name, int address, QString type )
{
    m_typeTable[ name ] = type;
    m_varsTable[ name ] = address;
}

void RamTable::addVariable( QString name, QString type )
{
    m_varNames.append( name );
    m_typeTable[ name ] = type;
    m_variableModel->appendRow( new QStandardItem(name) );
}

uint16_t RamTable::getCurrentAddr()
{
    int row = table->currentRow();
    if( row < 0 ) return 0;
    QString text = table->item( row, 0 )->text();
    if( text == "---" ) return -1;
    return  text.toUInt(NULL,16);
}

void RamTable::updateItems()
{
    for( int row=0; row<m_numRegs; row++ )
    {
        QTableWidgetItem* it = table->item( row, 1 );
        if( it->text() != "" ) addToWatch( it );
    }
}

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
    }
    else if( m_cpuMonitor )
    {
        QString type = "uint8";
        if( m_typeTable.contains( name ) ) type = m_typeTable.value( name );
        if( m_regNames.contains( name ) || m_varNames.contains( name ) )
        {
            watchList[_row] = name;
            table->item( _row, 2 )->setText( type );
            if( type == "string" )
            {
                table->item( _row, 3 )->setText(" - ");
                table->item( _row, 4 )->setText("");
            }else{
                table->item( _row, 3 )->setText("0");
                table->item( _row, 4 )->setText("0000 0000");
            }
        }
    }else{
        int addr = -1;
        if( m_varsTable.contains( name ) ) addr = m_varsTable.value( name );
        else                               addr = m_processor->getRegAddress( name );

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

            int indx = m_varNames.indexOf( name );
            int listEnd = m_varNames.size()-1;
            for( int i=1; i<size ; i++ )
            {
                int index = indx+i;
                if( index > listEnd ) break;

                QString varName = m_varNames.at( index );
                if( varName.contains( name ) ) table->item( _row+i, 1 )->setText( varName );
}   }   }   }

void RamTable::updateValues()
{
    if( !m_processor ) return;

    for( int _row: watchList.keys() )
    {
        m_currentRow = _row;
        QString name = watchList[_row];

        if( m_cpuMonitor  )
        {
            QString type = "";
            if( m_typeTable.contains( name ) ) type = m_typeTable.value( name );
            if( type == "string" )
            {
                setItemValue( 4, m_processor->cpu->getStrReg( name ) );
            }
            else{
                int value = m_processor->cpu->getCpuReg( name );
                if( value < 0 ) continue;
                setItemValue( 3, value );
                if( type.contains("8") ) setItemValue( 4, decToBase(value, 2, 8) );
                else setItemValue( 4, "0x"+decToBase(value, 16, 4).remove(0,1) );
            }
        }else{
            bool ok;
            int addr = name.toInt(&ok, 10);
            if( !ok && name.startsWith("0x") ) addr = name.toInt(&ok, 16);

            if( ok ){                              // Address
                uint8_t value = m_processor->getRamValue( addr );
                table->item( _row, 2 )->setText("u8");
                table->item( _row, 3 )->setData( 0, value );
                table->item( _row, 4 )->setData( 0, decToBase(value, 2, 8) );
            }
            else updateRamValue( name );  // Var or Reg name
        }
}   }

void RamTable::updateRamValue( QString name )
{
    //name = name.toUpper();
    QString type = "u8";
    if( m_typeTable.contains( name ) ) type = m_typeTable.value( name );
    setItemValue( 2, type );

    /*if( m_cpuMonitor  )
    {
        int value = m_processor->cpu->getCpuReg( name );
        setItemValue( 3, value );
        setItemValue( 4, decToBase(value, 2, 8) );
    }
    else*/
    {
        QByteArray ba;
        ba.resize(4);

        int address = -1;
        if( m_varsTable.contains( name ) ) address = m_varsTable.value( name );
        else                               address = m_processor->getRegAddress( name );
        if( address < 0 ) return;

        int bits = 8;

        if( type.contains( "32" ) )    // 4 bytes
        {
            bits = 32;
            ba[0] = m_processor->getRamValue( address );
            ba[1] = m_processor->getRamValue( address+1 );
            ba[2] = m_processor->getRamValue( address+2 );
            ba[3] = m_processor->getRamValue( address+3 );
        }
        else if( type.contains( "16" ) )  // 2 bytes
        {
            bits = 16;
            ba[0] = m_processor->getRamValue( address );
            ba[1] = m_processor->getRamValue( address+1 );
            ba[2] = 0;
            ba[3] = 0;
        }else{                             // 1 byte
            ba[0] = m_processor->getRamValue( address );
            ba[1] = 0;
            ba[2] = 0;
            ba[3] = 0;
        }
        if( type.contains( "f" ) )        // float, double
        {
            float value = 0;
            memcpy( &value, ba, 4 );
            setItemValue( 2, value );
        }
        else{                             // char, int, long
            int32_t value = 0;

            if( type.contains( "u" ) )
            {
                uint32_t val = 0;
                memcpy(&val, ba, 4);
                value = val;
            }else{
                if( bits == 32 )
                {
                    int32_t val = 0;
                    memcpy(&val, ba, 4);
                    value = val;
                }
                else if( bits == 16 )
                {
                    int16_t val = 0;
                    memcpy(&val, ba, 2);
                    value = val;
                }else{
                    int8_t val = 0;
                    memcpy(&val, ba, 1);
                    value = val;
            }   }
            setItemValue( 3, value  );

            if     ( type.contains( "8" ) ) setItemValue( 4, decToBase(value, 2, 8)  );
            else if( type.contains( "string" ) )
            {
                QString strVal = "";
                for( int i=address; i<=address+value; i++ )
                {
                    QString str = "";
                    const QChar cha = m_processor->getRamValue( i );
                    str.setRawData( &cha, 1 );

                    strVal += str; //QByteArray::fromHex( getRamValue( i ) );
                }
                setItemValue( 4, strVal );
        }   }
    }
}
