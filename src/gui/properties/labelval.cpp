/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "labelval.h"
#include "mainwindow.h"

LabelVal::LabelVal( QWidget* parent )
        : QWidget( parent )
{
    setupUi( this );

    //float scale = MainWindow::self()->fontScale();
    //QFont font = label->font();
    //font.setPixelSize( 11.0*scale );
    //label->setFont( font );
}

void LabelVal::setLabelVal( QString caption )
{
    label->setText( caption );
}

