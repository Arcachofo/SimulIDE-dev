/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "serialmon.h"
#include "mcuinterface.h"
#include "simulator.h"

SerialMonitor::SerialMonitor( QWidget* parent, McuInterface* mcu, int uartNum )
             : QDialog( parent )
             , m_uartInPanel(this)
             , m_uartOutPanel(this)
{
    setupUi(this);

    panelLayout->addWidget( &m_uartOutPanel );
    panelLayout->addWidget( &m_uartInPanel );

    m_processor = mcu;
    m_uartNum = uartNum;
    m_printASCII = true;
    m_addCR = false;

    Simulator::self()->addToUpdateList( &m_uartOutPanel );
    Simulator::self()->addToUpdateList( &m_uartInPanel );
}


void SerialMonitor::on_text_returnPressed()
{
    QByteArray array = text->text().toUtf8();

    for( int i=0; i<array.size(); i++ )
        m_processor->uartIn( m_uartNum, array.at(i) );

    if( m_addCR ) m_processor->uartIn( m_uartNum, 13 );
}

void SerialMonitor::on_value_returnPressed()
{
    QString text = value->text();

    m_processor->uartIn( m_uartNum, text.toInt() );
}

void SerialMonitor::on_valueButton_clicked()
{
    m_printASCII = !valueButton->isChecked();
    asciiButton->setChecked( m_printASCII );
}

void SerialMonitor::on_asciiButton_clicked()
{
    m_printASCII = asciiButton->isChecked();
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
