/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QVBoxLayout>

#include "voltwidget.h"
#include "custombutton.h"
#include "customdial.h"
#include "customslider.h"

VoltWidget::VoltWidget()
{
    DialWidget::setSize( 36 );
    m_dial->setValue( 0 );

    pushButton = new CustomButton( this );
    pushButton->setCheckable( true );

    QFont font;
    font.setFamily("Ubuntu");
    font.setPixelSize(8);
    font.setLetterSpacing( QFont::PercentageSpacing, 100 );
#ifdef _WIN32
    font.setLetterSpacing( QFont::PercentageSpacing, 90 );
#endif
    pushButton->setFont( font );

    m_verticalLayout->addWidget( pushButton );
    m_verticalLayout->setAlignment( pushButton, Qt::AlignHCenter );

    //setFixedSize( 60, 90 );
}
VoltWidget::~VoltWidget() {}
