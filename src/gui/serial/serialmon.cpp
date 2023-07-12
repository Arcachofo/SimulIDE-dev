/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "serialmon.h"
#include "usartmodule.h"
#include "simulator.h"
#include "utils.h"

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
    m_printMode = 0;
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

void SerialMonitor::on_printBox_currentIndexChanged( int index )
{
    //qDebug() << index;
    m_printMode = index;
}

void SerialMonitor::printIn( int value ) // Receive one byte on Uart
{
    m_uartInPanel.appendText( valToString( value ) );
}

void SerialMonitor::printOut( int value ) // Send value to OutPanelText
{
    m_uartOutPanel.appendText( valToString( value ) );
}

QString SerialMonitor::valToString( int val )
{
    uint8_t byte = val & 0xFF;

    QString text = "";

    switch ( m_printMode )
    {
        case 0: text.append( byte );                 break; // ASCII
        case 1: text = decToBase( byte, 16, 2 )+" "; break; // HEX
        case 2: text = decToBase( byte, 10, 3 )+" "; break; // DEC
        case 3: text = decToBase( byte,  8, 3 )+" "; break; // OCT
        case 4: text = decToBase( byte,  2, 8 )+" ";        // BIN
    }
    return text;
}

void SerialMonitor::closeEvent( QCloseEvent *event )
{
    event->accept();
    m_usart->monitorClosed();
}
