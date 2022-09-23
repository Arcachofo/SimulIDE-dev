/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QVariant>
#include <QDial>
#include <QVBoxLayout>

#include "dialwidget.h"

DialWidget::DialWidget()
{
}
DialWidget::~DialWidget() {}

void DialWidget::setupWidget()
{
    dial = new QDial(this);
    dial->setObjectName("dial");
    dial->setNotchesVisible(true);

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(0);
    verticalLayout->addWidget(dial);
}

#include "moc_dialwidget.cpp"
