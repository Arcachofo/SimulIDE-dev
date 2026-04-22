/***************************************************************************
 *   Copyright (C) 2023 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QCloseEvent>

#include "managecomps.h"
#include "componentlist.h"
#include "treeitem.h"
#include "mainwindow.h"
#include "thememanager.h"

manCompDialog::manCompDialog( QWidget* parent )
             : QDialog( parent )
{
    setupUi(this);

    m_initialized = false;

    float scale = MainWindow::self()->fontScale();

    table->verticalHeader()->hide(); //setVisible(False)
    table->verticalHeader()->setDefaultSectionSize( 20.0*scale );
    table->setHorizontalHeaderLabels( QStringList()<<tr("Component")<<tr("ShortCut") );
    table->setColumnWidth( 0, 140.0*scale );
    table->setColumnWidth( 1, 60.0*scale );

    connect( table, &QTableWidget::itemChanged,
              this, &manCompDialog::slotItemChanged, Qt::UniqueConnection );
}

void manCompDialog::addItem( TreeItem* treeItem )
{
    QTableWidgetItem* listItem = new QTableWidgetItem();
    listItem->setFlags( Qt::ItemIsEnabled );
    listItem->setText( treeItem->nameTr() );

    //if( treeItem->isHidden() ) listItem->setCheckState( Qt::Unchecked );
    //else                       listItem->setCheckState( Qt::Checked );

    QTableWidgetItem* shortItem = new QTableWidgetItem();
    shortItem->setText( treeItem->shortcut() );

    int row = table->rowCount();
    table->insertRow( row );
    table->setItem( row, 0, listItem );
    table->setItem( row, 1, shortItem );

    m_treeToList[ listItem ]   = treeItem;
    m_treeToShort[ shortItem ] = treeItem;
    //m_treeToShort.key()

    int childCount = treeItem->childCount();
    if( childCount > 0 )
    {
        listItem->setFlags( Qt::NoItemFlags );
        shortItem->setFlags( Qt::NoItemFlags );

        listItem->setBackground( QColor(240, 235, 245) );
        listItem->setForeground( QBrush( QColor( 110, 95, 50 )));

        for( int i=0; i<childCount; i++ ) addItem( (TreeItem*)treeItem->child( i ) );
    }
    else listItem->setIcon( ThemeManager::self()->icon(":/blanc.png") );
}

void manCompDialog::initialize( TreeItem* treeItem )
{
    if( !m_initialized )
    {
        QList<QTreeWidgetItem*> itemList = ComponentList::self()->findItems("",Qt::MatchStartsWith);

        for( QTreeWidgetItem* item : itemList ) addItem( (TreeItem*)item );

        //addInstallItem("PIC; 14 bit microcontrollers.; PIC.zip; 2507102250", 0 );
        //checkForUpdates();

        m_initialized = true;
    }
    QTableWidgetItem* item = m_treeToShort.key( treeItem );
    QTableWidgetItem* lItem = m_treeToList.key( treeItem );

    for( QTableWidgetItem* listItem : m_treeToList.keys() )
    {
        if( listItem->flags() == 0 ) continue;
        if( treeItem && listItem == lItem ) listItem->setBackground( QColor(255, 235, 155) );
        else                                listItem->setBackground( QColor(255, 255, 255) );
    }

    if( !treeItem ) return;

    if( item->flags() != 0 ) table->editItem( item );

    table->scrollToItem( item );
}

void manCompDialog::slotItemChanged( QTableWidgetItem* item )
{
    if( !m_initialized ) return;

    if( item->column() == 0 )  // Show/Hide
    {
        //TreeItem* treeItem = m_treeToList[ item ];

        //bool visible = item->checkState();
        //treeItem->setItemHidden( !visible );

        //for( int i=0; i<treeItem->childCount(); ++i )
        //{
        //    TreeItem*         childItem = (TreeItem*)treeItem->child( i );
        //    QTableWidgetItem* listItem  = m_treeToList.keys( childItem ).at(0);

        //    //if( visible ) listItem->setCheckState( Qt::Checked );
        //    //else          listItem->setCheckState( Qt::Unchecked );
        //}
    }else                      // Shortcut
    {
        TreeItem* treeItem = m_treeToShort[ item ];
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
