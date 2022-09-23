/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QVariant>
#include <QDial>
#include <QPushButton>
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
    
    pushButton = new QPushButton(this);
    pushButton->setObjectName( QString::fromUtf8("pushButton") );
    pushButton->setCheckable(true);
    pushButton->setForegroundRole( QPalette::BrightText );
    pushButton->setFixedSize( 40, 17 );

    QFont sansFont("Helvetica [Cronyx]", 8 );
    sansFont.setPixelSize(8);
    pushButton->setFont( sansFont );
    
    verticalLayout->addWidget(pushButton);
    verticalLayout->setAlignment( pushButton, Qt::AlignHCenter );
}

