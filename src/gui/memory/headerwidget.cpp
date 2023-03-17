/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QLabel>
#include <math.h>

#include "headerwidget.h"
#include "mainwindow.h"

HeaderWidget::HeaderWidget( QString name, QString type, QWidget* parent )
            : QWidget( parent )
{
    setupUi(this);

    m_name = name;
    m_type = type;

    nameLabel->setContentsMargins( 2, 2, 2, 6 );
    typeLabel->setContentsMargins( 2, 2, 2, 6 );
    valueLine->setContentsMargins( 2, 2, 2, 6 );

    nameLabel->setText( m_name );
    typeLabel->setText( m_type );
    valueLine->setText("Value");

    float scale = MainWindow::self()->fontScale();

    //typeLabel->setFixedWidth( round(40*scale) );
    valueLine->setFixedWidth( round(120*scale) );
}

void HeaderWidget::setValueStr( QString str )
{
    valueLine->setText( str );
}
