/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "serialmon.h"
#include "usartmodule.h"
#include "simulator.h"

SerialMonitor::SerialMonitor( QWidget* parent, UsartModule* usart )
             : QDialog( parent )
             , Updatable()
             , m_uartInPanel(this)
             , m_uartOutPanel(this)
{
    setupUi(this);

    m_uartInPanel.setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_uartOutPanel.setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    panelLayout->addWidget( &m_uartInPanel );
    panelLayout->addWidget( &m_uartOutPanel );

    m_usart = usart;
    m_printASCII = true;
    m_addCR = false;

    Simulator::self()->addToUpdateList( this );
}

void SerialMonitor::updateStep()
{
    m_uartInPanel.updateStep();
    m_uartOutPanel.updateStep();

    if( m_outBuffer.isEmpty() ) return;

    for( int i=0; i<m_outBuffer.size(); i++ )
        m_usart->uartIn( m_outBuffer.at(i) );

    m_outBuffer.clear();
}

void SerialMonitor::on_text_returnPressed()
{
    m_outBuffer.append( text->text().toLocal8Bit() );

    if( m_addCR ) m_outBuffer.append( 13 );//    m_usart->uartIn( 13 );
}

void SerialMonitor::on_value_returnPressed()
{
    m_outBuffer.append( value->text().toInt() );
}

void SerialMonitor::on_valueButton_clicked()
{
    m_printASCII = !valueButton->isChecked(); // Button is not yet checked
    asciiButton->setChecked( m_printASCII );
}

void SerialMonitor::on_asciiButton_clicked()
{
    m_printASCII = asciiButton->isChecked();  // Button is not yet checked
    valueButton->setChecked( !m_printASCII );
}

void SerialMonitor::printIn( int value ) // Receive one byte on Uart
{
    uint8_t byte = value & 0xFF;

    QString text = "";
    if( m_printASCII )
    {
        if( value == 0 ) return;
        text.append( byte );
    }
    else text = QString::number( byte )+" ";

    m_uartInPanel.appendText( text );
}

void SerialMonitor::printOut( int value ) // Send value to OutPanelText
{
    uint8_t byte = value & 0xFF;

    QString text = "";
    if( m_printASCII ) text.append( byte );
    else               text = QString::number( byte )+" ";

    m_uartOutPanel.appendText( text );
}

void SerialMonitor::closeEvent( QCloseEvent *event )
{
    event->accept();
    m_usart->monitorClosed();
}
