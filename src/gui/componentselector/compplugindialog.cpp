/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
 
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QCloseEvent>

#include "compplugindialog.h"
#include "componentselector.h"
#include "mainwindow.h"
#include "ui_compplugin.h"

CompPluginDialog::CompPluginDialog( QWidget *parent )
                : QDialog( parent )
                , ui( new Ui::ComponentPlugins )
{
    ui->setupUi(this);

    m_compList = ui->compList;
    m_initialized = false;

    connect( m_compList, SIGNAL( itemChanged( QListWidgetItem* )),
                   this, SLOT( slotItemChanged( QListWidgetItem* )), Qt::UniqueConnection );
}

CompPluginDialog::~CompPluginDialog() { delete ui; }

void CompPluginDialog::addItem( QTreeWidgetItem* item )
{
    QString itemName = item->text( 0 );

    QListWidgetItem* listItem = new QListWidgetItem( itemName );
    listItem->setCheckState( item->isHidden() ? Qt::Unchecked : Qt::Checked );

    m_compList->addItem( listItem );
    m_qtwItem[ listItem ] = item;

    int childCount = item->childCount();
    if( childCount > 0 )
    {
        listItem->setBackgroundColor( QColor(240, 235, 245));
        listItem->setForeground( QBrush( QColor( 110, 95, 50 )));

        for( int i=0; i<childCount; i++ ) addItem( item->child( i ) );
    }
    else listItem->setIcon( QIcon(":/blanc.png") );
}

void CompPluginDialog::setPluginList()
{
    if( m_initialized ) return;

    QList<QTreeWidgetItem*> itemList = ComponentSelector::self()->findItems("",Qt::MatchStartsWith);

    for( QTreeWidgetItem* item : itemList ) addItem( item );

    m_initialized = true;

    /*QDir pluginsDir( qApp->applicationDirPath() );

    pluginsDir.cd( "plugins" );
    QStringList dirList = pluginsDir.entryList( QDir::Dirs );

    for( QString  dirName : dirList )
    {
        if( dirName.contains(".") ) continue;

        QString pluginName = dirName.remove("plugin");

        if( m_compList->findItems( pluginName, Qt::MatchCaseSensitive ).isEmpty() )
        {
            QListWidgetItem* listItem = new QListWidgetItem( pluginName );

            QFile fIn( "data/plugins/"+pluginName.toUpper()+"uninstall" );
            if( fIn.open( QFile::ReadOnly | QFile::Text ) )
            {
                listItem->setCheckState(Qt::Checked);
                fIn.close();
            }
            else
            {
                listItem->setCheckState(Qt::Unchecked);
            }

            m_compList->addItem( listItem );
        }
    }*/
}

void CompPluginDialog::slotItemChanged( QListWidgetItem* item )
{
    if( !m_initialized ) return;
    setItemVisible( item, item->checkState() );
}

void CompPluginDialog::setItemVisible( QListWidgetItem* listIt, bool visible )
{
    QTreeWidgetItem* treeItem = m_qtwItem[ listIt ];
    treeItem->setHidden( !visible );

    int childCount = treeItem->childCount();
    if( childCount > 0 )
    {
        for( int i=0; i<childCount; i++ )
        {
            QListWidgetItem* listItem = m_qtwItem.keys( treeItem->child( i ) ).at(0);

            if( visible ) listItem->setCheckState( Qt::Checked );
            else          listItem->setCheckState( Qt::Unchecked );
}   }   }

void CompPluginDialog::reject()
{
    this->setVisible( false );

    for( QListWidgetItem* listItem : m_compList->findItems("",Qt::MatchStartsWith) )
    {
        bool hidden = ( listItem->checkState() == Qt::Unchecked );
        MainWindow::self()->settings()->setValue( listItem->text()+"/hidden", hidden );
}   }

#include "moc_compplugindialog.cpp"
