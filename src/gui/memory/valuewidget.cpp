/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

//#include <math.h>

#include "valuewidget.h"
#include "mainwindow.h"
#include "e_mcu.h"
#include "cpubase.h"
#include "utils.h"

ValueWidget::ValueWidget( QString name, QString type, eMcu* processor, QWidget* parent )
           : QWidget( parent )
{
    setupUi(this);

    m_name = name;
    m_type = type.toLower();
    m_processor = processor;

    float scale = MainWindow::self()->fontScale();
    QFont fontS;
    fontS.setFamily("Ubuntu Mono");
    fontS.setBold( true );
    fontS.setPixelSize( round(12.5*scale) );

    QFont font;
    font.setFamily("Ubuntu Mono");
    font.setBold( true );
    font.setPixelSize( round(12.5*scale) );

    nameLabel->setFont( font );
    nameLabel->setText( m_name );

    typeLabel->setFont( fontS );
    typeLabel->setText( m_type );
    //typeLabel->setFixedWidth( round(40*scale) );

    valueLine->setFixedWidth( round(120*scale) );
    valueLine->setReadOnly( true );
}

void ValueWidget::updateValue()
{
    if( !m_processor ) return;

    if( m_type == "string" ) setValueStr( m_processor->cpu->getStrReg( m_name ) );
    else                     setValueInt( m_processor->cpu->getCpuReg( m_name ) );
}

void ValueWidget::setValueInt( int val )
{
    if( m_type.endsWith("8") )        // 8 bits
    {
        val = val  & 0xFF;
        QString decStr = decToBase( val, 10, 3 );
        QString hexStr = decToBase( val, 16, 2 );
        QString binStr = decToBase( val,  2, 8 );
        m_strVal = decStr+" 0x"+hexStr+" "+binStr;
    }
    else if( m_type.endsWith("16") ) // 16 bits
    {
        val = val  & 0xFFFF;
        QString decStr = decToBase( val, 10, 5 );
        QString hexStr = decToBase( val, 16, 4 );
        m_strVal = decStr+"  0x"+hexStr;
    }
    else                            // Unknown
    {
        m_strVal = QString::number(val);
    }
    valueLine->setText( m_strVal );
}

void ValueWidget::setValueStr( QString str )
{
    valueLine->setText( str );
}
