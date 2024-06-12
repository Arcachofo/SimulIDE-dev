/***************************************************************************
 *   Copyright (C) 2023 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QSettings>
#include <QCloseEvent>

#include "managecomps.h"
#include "treeitem.h"
#include "componentlist.h"
#include "mainwindow.h"

manCompDialog::manCompDialog( QWidget* parent )
             : QDialog( parent )
{
    setupUi(this);

    m_initialized = false;

    table->verticalHeader()->hide(); //setVisible(False)
    table->setHorizontalHeaderLabels( QStringList()<<tr("Name")<<tr("ShortCut") );
    table->setColumnWidth( 0, 300 );
    table->setColumnWidth( 1, 90 );

    connect( table, &QTableWidget::itemChanged,
              this, &manCompDialog::slotItemChanged, Qt::UniqueConnection );
}

void manCompDialog::addItem( TreeItem* treeItem )
{
    QTableWidgetItem* listItem = new QTableWidgetItem();
    listItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    listItem->setText( treeItem->nameTr() );

    if( treeItem->isHidden() ) listItem->setCheckState( Qt::Unchecked );
    else                   listItem->setCheckState( Qt::Checked );

    QTableWidgetItem* shortItem = new QTableWidgetItem();
    shortItem->setText( treeItem->shortcut() );

    int row = table->rowCount();
    table->insertRow( row );
    table->setItem( row, 0, listItem );
    table->setItem( row, 1, shortItem );

    m_treeItems[ listItem ]  = treeItem;
    m_treeItems[ shortItem ] = treeItem;

    int childCount = treeItem->childCount();
    if( childCount > 0 )
    {
        listItem->setBackground( QColor(240, 235, 245) );
        listItem->setForeground( QBrush( QColor( 110, 95, 50 )));

        for( int i=0; i<childCount; i++ ) addItem( (TreeItem*)treeItem->child( i ) );
    }
    else listItem->setIcon( QIcon(":/blanc.png") );
}

void manCompDialog::initialize()
{
    if( m_initialized ) return;

    QList<QTreeWidgetItem*> itemList = ComponentList::self()->findItems("",Qt::MatchStartsWith);

    for( QTreeWidgetItem* item : itemList ) addItem( (TreeItem*)item );

    m_initialized = true;
}

void manCompDialog::slotItemChanged( QTableWidgetItem* item )
{
    if( !m_initialized ) return;

    TreeItem* treeItem = m_treeItems[ item ];

    if( item->column() == 0 )  // Hidden
    {
        bool visible = item->checkState();
        treeItem->setItemHidden( !visible );

        for( int i=0; i<treeItem->childCount(); ++i )
        {
            TreeItem*         childItem = (TreeItem*)treeItem->child( i );
            QTableWidgetItem* listItem  = m_treeItems.keys( childItem ).at(0);

            if( visible ) listItem->setCheckState( Qt::Checked );
            else          listItem->setCheckState( Qt::Unchecked );
        }
    }else              // Shortcut
    {
        QString text = item->text().left(1);
        item->setText( text );
        treeItem->setShortCut( text );
        ComponentList::self()->setShortcut( text, treeItem->name() );
    }
}

/*void manCompDialog::reject()
{
    this->setVisible( false );

    QHash<QString, QString> shortCuts;

    for( int row=0; row<table->rowCount(); row++ )
    {
        QTableWidgetItem* listItem = table->item( row, 0 );
        bool hidden = ( listItem->checkState() == Qt::Unchecked );
        QString name = m_treeItems.value( listItem )->name();
        MainWindow::self()->compSettings()->setValue( name+"/hidden", hidden );

        QString shortCut = table->item( row, 1 )->text();
        if( !shortCut.isEmpty() )
        {
            shortCuts[name] = shortCut;
            MainWindow::self()->compSettings()->setValue( name+"/shortcut", shortCut );
        }
    }
    ComponentList::self()->setShortCuts( shortCuts );
}*/
