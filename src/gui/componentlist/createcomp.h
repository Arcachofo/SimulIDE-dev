/***************************************************************************
 *   Copyright (C) 2024 by santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CREATECOMP_H
#define CREATECOMP_H

#include <QDialog>

#include "ui_createcomp.h"

class creCompDialog : public QDialog, private Ui::creCompDialog
{
    Q_OBJECT

    struct iconItem_t{
        QString iconFile;
        QString iconData;
    };

    public:
        creCompDialog( QWidget* parent=0 );

        QString toString();

    public slots:
        void on_iconChoose_clicked();

    private slots:
        void accept();
        void reject();

    private:
        void addIcon( iconItem_t iconItem );
        void addIconFile( QString iconFile );

        QString m_circuitPath;
        QString m_iconFile;

        QList<iconItem_t> m_itemList;
};
#endif
