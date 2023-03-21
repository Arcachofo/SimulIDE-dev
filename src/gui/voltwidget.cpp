/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDial>
#include "custombutton.h"
#include <QVBoxLayout>

#include "voltwidget.h"

VoltWidget::VoltWidget()
{
    setupWidget();
    setFixedSize( 60, 90 );
}
VoltWidget::~VoltWidget() {}

void VoltWidget::setupWidget()
{
    DialWidget::setupWidget();
    
    dial->setMinimum(0);
    dial->setMaximum(500);
    dial->setValue(000);
    dial->setSingleStep(10);
    
    pushButton = new CustomButton(this);
    pushButton->setCheckable(true);

    QFont font;
    font.setFamily("Ubuntu");
    font.setPixelSize(8);
    font.setLetterSpacing( QFont::PercentageSpacing, 100 );
#ifdef _WIN32
    font.setLetterSpacing( QFont::PercentageSpacing, 90 );
#endif
    pushButton->setFont( font );
    
    verticalLayout->addWidget( pushButton );
    verticalLayout->setAlignment( pushButton, Qt::AlignHCenter );
}

