/***************************************************************************
 *   Copyright (C) 2025 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QSettings>
#include <QPushButton>
#include <QNetworkReply>

#include "installer.h"
#include "componentlist.h"
#include "installitem.h"
#include "mainwindow.h"
#include "qzipreader.h"
#include "utils.h"

Installer::Installer( QWidget* parent )
         : QWidget( parent )
{
    setupUi(this);

    m_checkUpdates = true; //false;
    m_listLoaded = false;
    m_changed = false;

    m_installItem = nullptr;

    installTable->horizontalHeader()->setStretchLastSection( true );

    m_compsUrl = "https://simulide.com/p/direct_downloads/components/";

    m_compsDir.setPath( MainWindow::self()->getConfigPath("components") );
    if( !m_compsDir.exists() ) m_compsDir.mkpath(".");

    loadList();

    QSettings* settings = MainWindow::self()->settings();
    QStringList installedList = settings->value("library/installed").toString().split(",");

    for( QString itemStr : installedList )
    {
        if( itemStr.isEmpty() ) continue;

        QStringList list = itemStr.split(";");
        if( list.size() < 2 ) return;
        QString name = list.first();

        InstallItem* item = m_items.value( name );
        if( !item ) continue;

        QString version = "2507102250" ;//list.at(1);//
        int64_t v = version.toLongLong();
        item->m_version = v;
        item->shouldUpdate( v );

        qDebug() << "Installer::Installer" << name << version;

        m_installed.insert( item->m_name, v );
    }
}

void Installer::loadList()
{
    QString compFile = m_compsDir.filePath("components.txt");
    QString replyStr = fileToString( compFile, "Installer::loadList" );

    QStringList setList = replyStr.split("\n"); // List of Component Sets

    m_changed = false;
    m_version = 0;
    int row = 0;
    for( QString itemStr : setList )
    {
        if( itemStr.isEmpty() ) continue;
        //qDebug() <<"Installer::loadList"<< itemStr;

        InstallItem* item = nullptr;

        QStringList list = itemStr.split(";");
        QString name = list.first();

        if( m_installed.contains( name ) )
        {
            item = m_items.value( name );

            if( item && list.size() > 3 ) {
                QString versionStr = list.at(3);
                versionStr.remove(0,1);
                //qDebug() << "Installer::loadList updated" << name << v << item->m_version;

                uint64_t version = versionStr.toLongLong();
                if( item->shouldUpdate( version ) ) m_changed = true;
                if( version > m_version ) m_version = version;
            }
        }
        else if( m_items.contains( name ) )
        {
            item = m_items.value( name );
        }
        else
        {
            item = new InstallItem( this, itemStr );

            item->shouldUpdate( 0 );

            m_items.insert( item->m_name, item );

            installTable->insertRow( row );
            installTable->setCellWidget( row, 0, item );

            m_changed = true;
        }
        float scale = MainWindow::self()->fontScale();

        if( item->m_file.isEmpty() ) installTable->setRowHeight( row, 25*scale );
        else                         installTable->setRowHeight( row, 60*scale );

        row++;
    }
    installTable->setRowCount( row );
    m_listLoaded = true;
}

void Installer::loadInstalled()
{
    for( QString item : m_installed.keys() )
    {
        QDir compSetDir = m_compsDir;
        compSetDir.cd( item );
        ComponentList::self()->LoadCompSetAt( compSetDir );
        QStringList itemList = ComponentList::self()->getxmlItems();
        m_groupItemList.insert( item, itemList );
    }
}

void Installer::checkForUpdates( QString url )
{
    if( !m_checkUpdates ) return;

    //QString version = MainWindow::self()->settings()->value("library/version").toString();
    //qDebug() << "version" << version;
    qDebug() << "Checking for Updates...";

    if( url.isEmpty() ) url = m_compsUrl+"dloadset.php?file=components.txt";
    QNetworkRequest request( url );

    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute
                        , QNetworkRequest::NoLessSafeRedirectPolicy );

    request.setTransferTimeout( 5000 );

    m_reply = m_manager.get( request );

    QObject::connect( m_reply, &QNetworkReply::finished,
                     [=](){ updtReady(); } );
}

void Installer::updtReady()
{
    if( m_reply->error() == QNetworkReply::NoError )
    {
        QString compFile = m_compsDir.filePath( "components.txt" );
        QFile file( compFile );
        if( file.exists() ) QFile::remove( compFile );

        if( file.open( QIODevice::WriteOnly | QIODevice::Append ))
        {
            file.write( m_reply->readAll() );
            file.close();

            loadList();

            if( m_changed ) qDebug() << "Installer: Updates available";
            else            qDebug() << "Installer: Up to date";
        }else{
            qDebug() << "Installer::updtReady ERROR: can't write file" << compFile ;
            m_installItem = nullptr;
        }
    }
    else qDebug() << "Installer::updtReady ERROR:" << m_reply->errorString(); // There was a network error

    m_reply->close();
}

void Installer::installItem( QString itemName )
{
    InstallItem* item = m_items.value( itemName );

    if( !item->m_depends.isEmpty() && !m_installed.contains( item->m_depends ) )
    {
        m_nextItem = itemName;
        itemName = item->m_depends;
        item = m_items.value( itemName );
    }
    else m_nextItem.clear();

    if( !item ) return;

    qDebug() << "Installing Component Set:" << itemName;

    QString url = m_compsUrl+"dloadset.php?file="+item->m_file;
    QNetworkRequest request( url );

    request.setAttribute( QNetworkRequest::RedirectPolicyAttribute
                        , QNetworkRequest::NoLessSafeRedirectPolicy );

    request.setTransferTimeout( 5000 );

    m_reply = m_manager.get( request );

    QObject::connect( m_reply, &QNetworkReply::finished,
                     [=](){ itemDataReady(); } );

    m_installItem = item;
}

void Installer::unInstallItem( QString itemName )
{
    QDir compSetDir = m_compsDir;
    compSetDir.cd( itemName );
    compSetDir.removeRecursively();
    m_installed.remove( itemName );

    ComponentList::self()->createList();
}

void Installer::itemDataReady()
{
    bool error = true;
    if( m_reply->error() == QNetworkReply::NoError )
    {
        error = false;
    }
    else qDebug() << "Installer::itemDataReady QNetworkReply ERROR"; // There was a network error

    m_reply->close();

    if( !error )
    {
        QString zipFile = m_compsDir.filePath( m_installItem->m_file );

        QFile file( zipFile );
        if( file.exists() ) QFile::remove( zipFile );

        if( file.open( QIODevice::WriteOnly | QIODevice::Append ))
        {
            file.write( m_reply->readAll() );
            file.close();
        }else{
            qDebug() << "Installer::itemDataReady ERROR: can't write file" << zipFile ;
            m_installItem = nullptr;
            return;
        }

        //QString setDir =m_compsDir.absolutePath()+ m_installItem->m_name;
        qZipReader qZip( zipFile );
        bool isExtracted = qZip.extractAll( m_compsDir.absolutePath() );

        if( isExtracted )
        {
            QDir compSetDir = m_compsDir;
            compSetDir.cd( m_installItem->m_name );
            ComponentList::self()->LoadCompSetAt( compSetDir );

            m_installed.insert( m_installItem->m_name, m_installItem->m_versionNext );
            qDebug() << m_installItem->m_name <<"Installed";
        }
        else qDebug() << "Installer::itemDataReady ERROR extracting" << zipFile ;

        QFile::remove( zipFile );
    }
    m_installItem = nullptr;
    if( !m_nextItem.isEmpty() ) installItem( m_nextItem );
}

void Installer::on_updtButton_clicked()
{
    checkForUpdates();
}

void Installer::writeSettings()
{
    QString installed; // = "Arduino,AVR,PIC,MCS51,MCS65,Z80,Analog,74,Digipot,Tools";

    for( QString itemName : m_installed.keys() )
    {
        InstallItem* item = m_items.value( itemName );
        installed += item->toString()+",";
    }
    QSettings* settings = MainWindow::self()->settings();
    settings->setValue("library/installed", installed );
}

