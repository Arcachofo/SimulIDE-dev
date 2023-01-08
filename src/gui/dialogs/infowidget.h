/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>

#include "ui_infowidget.h"

class InfoWidget : public QWidget, private Ui::infoWidget
{
    Q_OBJECT

    public:
        InfoWidget( QWidget* parent=0 );

        void setRate( double rate , int load );
        void setCircTime( uint64_t tStep );
        void setTargetSpeed( double s );

    public slots:

    private:

};

#endif
