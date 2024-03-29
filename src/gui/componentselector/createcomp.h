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

    public:
        creCompDialog( QWidget* parent=0 );

    public slots:
        //void on_typeBox_currentIndexChanged( int index );
        //void on_categoryEdit_editingFinished();
        void on_iconChoose_clicked();
        //void on_iconImage_clicked();

    private slots:
        void accept();
        void reject();

    private:
        void embedIcon();
        void updtIconData();

        QString m_circuitPath;
        QString m_iconFile;
        QString m_iconData;
};
#endif
