/***************************************************************************
 *   Copyright (C) 2025 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include <QWidget>
#include <QDir>
#include <QNetworkAccessManager>

#include "ui_installer.h"

class InstallItem;
class QNetworkReply;

class Installer : public QWidget, private Ui::installer
{
    Q_OBJECT

    public:
        Installer( QWidget* parent );

        void checkForUpdates( QString url="" );

        void loadInstalled();

        void installItem( QString itemName );
        void unInstallItem( QString itemName );

        void writeSettings();

        QStringList getGroupItems( QString group ) { return m_groupItemList.value( group ); }

    public slots:
        void on_updtButton_clicked();

    private:
        //void addItemInstaller( QString itemStr, int row );
        void updtReady();
        void itemDataReady();
        void loadList();

        bool m_checkUpdates;
        bool m_listLoaded;
        bool m_changed;

        QDir m_compsDir;
        QString m_compsUrl;
        uint64_t m_version;

        QString m_nextItem;

        QMap<QString, InstallItem*> m_items;
        QMap<QString, int64_t> m_installed;
        QMap<QString, QStringList>  m_groupItemList;

        InstallItem* m_installItem;

        QNetworkReply* m_reply;
        QNetworkAccessManager m_manager;
};
