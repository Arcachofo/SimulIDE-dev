/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDialog>

#include "ui_about.h"

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H


class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT

    public:
        AboutDialog( QWidget* parent=0 );

    public slots:

};


#endif
