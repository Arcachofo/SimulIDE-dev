/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDial>
#include <QVBoxLayout>

#include "dialwidget.h"
#include "customdial.h"

DialWidget::DialWidget()
{
    setAttribute( Qt::WA_TranslucentBackground );
}
DialWidget::~DialWidget() {}

void DialWidget::setupWidget()
{
    dial = new CustomDial(this);
    dial->setObjectName("dial");
    dial->setNotchesVisible(true);

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(0);
    verticalLayout->addWidget(dial);
}

void DialWidget::paintEvent( QPaintEvent* e )
{
}

#include "moc_dialwidget.cpp"
