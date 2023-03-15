/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef HEADERWIDGET_H
#define HEADERWIDGET_H

#include <QWidget>

#include "ui_headerwidget.h"

class HeaderWidget : public QWidget, private Ui::HeaderWidget
{
    Q_OBJECT

    public:
        HeaderWidget( QString name, QString type, QWidget* parent=0 );

        void setValueStr( QString str );

    private:
        QString m_name;
        QString m_type;

        QString m_strVal;

};

#endif
