/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QStandardItemModel>
#include <math.h>

#include "watcher.h"
#include "valuewidget.h"
#include "headerwidget.h"
#include "mainwindow.h"
#include "e_mcu.h"
#include "cpubase.h"
#include "console.h"
#include "scriptcpu.h"

Watcher::Watcher( QWidget* parent, eMcu* processor )
       : QWidget( parent )
{
    setupUi(this);

    m_processor = processor;

    m_console= NULL;
    m_header = false;

    QFont font;
    float scale = MainWindow::self()->fontScale();
    font.setFamily("Ubuntu Mono");
    font.setBold( true );
    //font.setPointSize( 14 );
    font.setPixelSize( round(12.5*scale) );

    m_valuesLayout = new QBoxLayout( QBoxLayout::TopToBottom, this );
    m_valuesLayout->setMargin( 0 );
    m_valuesLayout->setSpacing( 2 );
    m_valuesLayout->setContentsMargins( 0, 0, 0, 0 );
    m_valuesLayout->addStretch();
    valuesWidget->setLayout( m_valuesLayout );

    m_registerModel = new QStandardItemModel( this );
    registers->setEditTriggers( QAbstractItemView::NoEditTriggers );
    registers->setModel( m_registerModel );
    registers->setFont( font );

    m_variableModel = new QStandardItemModel( this );
    variables->setEditTriggers( QAbstractItemView::NoEditTriggers );
    variables->setFont( font );
    variables->setModel( m_variableModel );

    splitter->setSizes( {{50,320}} );
    splitter_2->setSizes( {100,30} );

    connect( variables, &QListView::activated,
             this,      &Watcher::VarDoubleClick );

    connect( registers, &QListView::activated,
             this,      &Watcher::RegDoubleClick );
}

void Watcher::addHeader()
{
    m_header = true;
    HeaderWidget* header = new HeaderWidget("Name","Type", this );
    m_valuesLayout->insertWidget( 0, header );
}

void Watcher::updateValues()
{
    if( !m_processor ) return;

    for( ValueWidget* vw : m_values ) vw->updateValue();
}

void Watcher::setRegisters( QStringList regs )
{
    regs.sort();
    // m_regNames = regs;
    for( QString reg : regs ) addRegister( reg, "uint8" );
}

void Watcher::addRegister( QString name, QString type )
{
    if( !m_header ) addHeader();
    if( m_typeTable.keys().contains( name ) ) return;
    m_typeTable[ name ] = type;
    m_registerModel->appendRow( new QStandardItem(name) );
}

void Watcher::setVariables( QStringList vars )
{
    vars.sort();
   //m_varNames = vars;
    m_variableModel->clear();
    for( QString var : vars ) m_variableModel->appendRow( new QStandardItem(var) );
}

void Watcher::addVariable( QString name, QString type )
{
    if( !m_header ) addHeader();
    if( m_typeTable.keys().contains( name ) ) return;
    m_typeTable[ name ] = type;
    m_variableModel->appendRow( new QStandardItem(name) );
}

void Watcher::loadVarSet( QStringList varSet )
{

}

QStringList Watcher::getVarSet()
{
    //bool empty = true;
    QStringList varset;
    /*for( int row=m_numRegs-1; row>=0; --row )
    {
        QString name = table->item( row, 1 )->text();
        if( !name.isEmpty() ) empty = false;          // Find first non empty
        if( empty ) continue;
        varset.prepend( name );
    }*/
    return varset;
}

void Watcher::addConsole( ScriptCpu* s )
{
    if( m_console ) return;
    m_console = new Console( s );
    m_valuesLayout->insertWidget( m_valuesLayout->count()-1, m_console );
}

void Watcher::RegDoubleClick( const QModelIndex& index )
{
    QString name = m_registerModel->item(index.row())->text();
    if( m_values.keys().contains( name ) ) return;

    QString type = m_typeTable.value( name );

    ValueWidget* valwid = new ValueWidget( name, type, m_processor, this );
    m_values[name] = valwid;

    int last = m_console ? 2 : 1;
    m_valuesLayout->insertWidget( m_valuesLayout->count()-last, valwid );
}

void Watcher::VarDoubleClick( const QModelIndex& index )
{
    QString name = m_variableModel->item(index.row())->text();
    if( m_values.keys().contains( name ) ) return;

    QString type = m_typeTable.value( name );

    ValueWidget* valwid = new ValueWidget( name, type, m_processor, this );
    m_values[name] = valwid;

    int last = m_console ? 2 : 1;
    m_valuesLayout->insertWidget( m_valuesLayout->count()-last, valwid );
}
