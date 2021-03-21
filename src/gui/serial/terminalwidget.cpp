/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "terminalwidget.h"
#include "mcucomponent.h"
#include "baseprocessor.h"
#include "simulator.h"
#include "circuit.h"
#include "serialterm.h"

TerminalWidget::TerminalWidget( QWidget* parent, SerialTerm* ser )
              : QWidget( parent )
              ,m_verticalLayout(this)
              ,m_sendLayout()
              ,m_textLayout()
              ,m_sendText(this)
              ,m_sendValue(this)
              ,m_uartInPanel(this)
              ,m_uartOutPanel(this)
              ,m_ascciButton(this)
              ,m_valueButton(this)
              ,m_addCrButton(this)
              ,m_clearInButton(this)
              ,m_clearOutButton(this)
              ,m_uartBox(this)
{
    m_serComp = ser;

    m_printASCII = true;
    m_addCR = false;
    m_uart = 0;
    
    setMinimumSize( QSize(200, 200) );

    setWindowFlags( Qt::Window | Qt::WindowTitleHint | Qt::Tool
                  | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
    
    m_verticalLayout.setObjectName( "verticalLayout" );
    m_verticalLayout.setContentsMargins(0, 0, 0, 0);
    m_verticalLayout.setSpacing(4);

    QLabel* sendTextLabel = new QLabel(this);
    sendTextLabel->setText(tr("Send Text:"));
    
    QLabel* sendValueLabel = new QLabel(this);
    sendValueLabel->setText(tr("    Send Value:"));
    m_sendValue.setMaxLength( 3 );
    m_sendValue.setMaximumWidth(40);
    m_sendValue.setValidator( new QIntValidator(0, 255, this) );

    QLabel* printLabel = new QLabel(this);
    printLabel->setText(tr("    Print:"));

    m_ascciButton.setCheckable(true);
    m_ascciButton.setForegroundRole( QPalette::BrightText );
    m_ascciButton.setFixedSize( 50, 20 );
    m_ascciButton.setText( " ASCII " );
    m_ascciButton.setChecked( m_printASCII );

    m_valueButton.setCheckable(true);
    m_valueButton.setForegroundRole( QPalette::BrightText );
    m_valueButton.setFixedSize( 50, 20 );
    m_valueButton.setText( tr(" Value ") );
    m_valueButton.setChecked( !m_printASCII );

    m_addCrButton.setCheckable(true);
    m_addCrButton.setForegroundRole( QPalette::BrightText );
    m_addCrButton.setFixedSize( 30, 20 );
    m_addCrButton.setText( tr("CR") );
    m_addCrButton.setChecked( false );

    //m_clearInButton.setCheckable(true);
    m_clearInButton.setForegroundRole( QPalette::BrightText );
    m_clearInButton.setFixedSize( 50, 20 );
    m_clearInButton.setText( tr("Clear") );
    m_clearInButton.setChecked( false );

    //m_clearOutButton.setCheckable(true);
    m_clearOutButton.setForegroundRole( QPalette::BrightText );
    m_clearOutButton.setFixedSize( 50, 20 );
    m_clearOutButton.setText( tr("Clear") );
    m_clearOutButton.setChecked( false );

    //m_uartBox.setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    m_uartBox.setFixedWidth( 70 );
    m_uartBox.setMaximum( 6 );
    m_uartBox.setMinimum( 1 );
    m_uartBox.setPrefix( "Uart" );
    m_uartBox.setValue( 1 );
    
    m_sendLayout.setSpacing(4);
    m_sendLayout.setContentsMargins(2, 2, 4, 2);
    m_sendLayout.addWidget( sendTextLabel );
    m_sendLayout.addWidget( &m_sendText );
    m_sendLayout.addWidget( &m_addCrButton );
    m_sendLayout.addWidget( sendValueLabel );
    m_sendLayout.addWidget( &m_sendValue );
    m_sendLayout.addWidget( printLabel );
    m_sendLayout.addWidget( &m_ascciButton );
    m_sendLayout.addWidget( &m_valueButton );
    m_verticalLayout.addLayout( &m_sendLayout );
    
    /*QFrame* myFrame = new QFrame();
    myFrame->setFrameShape(QFrame::HLine);
    m_verticalLayout.addWidget( myFrame );*/
    
    QHBoxLayout* textLabelsLayoutI = new QHBoxLayout();
    QLabel* sentLabel = new QLabel(this);
    sentLabel->setText(tr("Received From Micro:"));
    textLabelsLayoutI->addWidget( &m_clearOutButton );
    textLabelsLayoutI->addWidget( sentLabel );

    QHBoxLayout* textLabelsLayoutO = new QHBoxLayout();
    QLabel* recvLabel = new QLabel(this);
    recvLabel->setText(tr("Sent to Micro:"));
    textLabelsLayoutO->addWidget( &m_clearInButton );
    textLabelsLayoutO->addWidget( recvLabel );
    textLabelsLayoutO->addWidget( &m_uartBox );

    QHBoxLayout* textLabelsLayout = new QHBoxLayout();
    textLabelsLayout->addLayout( textLabelsLayoutI );
    textLabelsLayout->addLayout( textLabelsLayoutO );
    m_verticalLayout.addLayout( textLabelsLayout );
    
    m_textLayout.addWidget( &m_uartOutPanel );
    m_textLayout.addWidget( &m_uartInPanel );
    m_verticalLayout.addLayout( &m_textLayout );

    Simulator::self()->addToUpdateList( &m_uartOutPanel );
    Simulator::self()->addToUpdateList( &m_uartInPanel );

    connect( &m_sendText, SIGNAL( returnPressed() ),
                    this, SLOT( onTextChanged() ), Qt::UniqueConnection);

    connect( &m_sendValue, SIGNAL( returnPressed() ),
                     this, SLOT( onValueChanged() ));

    connect( &m_ascciButton, SIGNAL( clicked()),
                       this, SLOT( ascciButtonClicked()), Qt::UniqueConnection );

    connect( &m_valueButton, SIGNAL( clicked()),
                       this, SLOT( valueButtonClicked()), Qt::UniqueConnection );

    connect( &m_addCrButton, SIGNAL( clicked()),
                       this, SLOT( addCRClicked()), Qt::UniqueConnection );

    connect( &m_clearInButton, SIGNAL( clicked()),
                         this, SLOT( clearInClicked()), Qt::UniqueConnection );

    connect( &m_clearOutButton, SIGNAL( clicked()),
                          this, SLOT( clearOutClicked()), Qt::UniqueConnection );

    connect( &m_uartBox, SIGNAL( valueChanged(int) ),
              m_serComp, SLOT( setUart(int) ), Qt::UniqueConnection );

    initialize();
}
TerminalWidget::~TerminalWidget()
{
    Simulator::self()->remFromUpdateList( &m_uartOutPanel );
    Simulator::self()->remFromUpdateList( &m_uartInPanel );
}

void TerminalWidget::setMcuId( QString mcu )
{
    QString name = Circuit::self()->origId( mcu );
    if( name == "" ) name = mcu;
    m_mcuId =  name;

    Component* mcuComp = Circuit::self()->getCompById( m_mcuId );
    if( mcuComp )
    {
        m_mcuComponent = static_cast<McuComponent*>(mcuComp);
        connectMcu();
    }
    else setWindowTitle( tr("Unknown Mcu") );

    //qDebug() << "TerminalWidget::setMcuId" << mcu << name;
}

void TerminalWidget::initialize()
{
    int circVersion = Circuit::self()->circType().split(".").last().toInt();

    if( circVersion < 5 )
    {
        m_mcuComponent = McuComponent::self();
        m_mcuId = m_mcuComponent->objectName();
        setMcuId( m_mcuId );
    }
}

void TerminalWidget::connectMcu()
{
    this->setWindowTitle( m_mcuComponent->idLabel() );

    m_processor = m_mcuComponent->processor();

    connect( m_mcuComponent, SIGNAL( closeSerials()),
                  m_serComp, SLOT(   slotClose()), Qt::UniqueConnection );

    connect( m_processor, SIGNAL( uartDataOut( int, int )),
                    this, SLOT(   uartOut( int, int )), Qt::UniqueConnection );

    connect( m_processor, SIGNAL( uartDataIn( int, int )),
                    this, SLOT(   uartIn( int, int )), Qt::UniqueConnection );
}

void TerminalWidget::closeEvent( QCloseEvent* event )
{
    //m_serComp->slotClose();
    Circuit::self()->removeComp( m_serComp );
    QWidget::closeEvent( event );
}

void TerminalWidget::onTextChanged()
{
    QString text = m_sendText.text();
    //qDebug()<< "TerminalWidget::onTextChanged" << text ;
    
    QByteArray array = text.toUtf8();
    
    for( int i=0; i<array.size(); i++ )
        m_processor->uartIn( m_uart, array.at(i) );

    if( m_addCR ) m_processor->uartIn( m_uart, 13 );
}

void TerminalWidget::onValueChanged()
{
    QString text = m_sendValue.text();

    m_processor->uartIn( m_uart, text.toInt() );
}

void TerminalWidget::valueButtonClicked()
{
    m_printASCII = !m_valueButton.isChecked();
    m_ascciButton.setChecked( m_printASCII );
}

void TerminalWidget::ascciButtonClicked()
{
    m_printASCII = m_ascciButton.isChecked();
    m_valueButton.setChecked( !m_printASCII );
}

void TerminalWidget::addCRClicked()
{
    m_addCR = m_addCrButton.isChecked();
}

void TerminalWidget::clearInClicked()
{
    m_uartInPanel.clear();
    //qDebug() << "TerminalWidget::clearInClicked";
}

void TerminalWidget::clearOutClicked()
{
    m_uartOutPanel.clear();
    //qDebug() << "TerminalWidget::clearOutClicked";
}

void TerminalWidget::uartChanged( int uart )
{
    m_uart = uart-1;
    if( m_uartBox.value() != uart ) m_uartBox.setValue( uart );
}

void TerminalWidget::uartIn( int uart, int value ) // Receive one byte on Uart
{
    //qDebug() << "TerminalWidget::uartIn" << m_uart << uart << value;
    if( uart != m_uart ) return;

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

void TerminalWidget::uartOut( int uart, int value ) // Send value to OutPanelText
{
    //qDebug() << "TerminalWidget::uartOut" << m_uart << uart << value;
    if( uart != m_uart ) return;

    uint8_t byte = value & 0xFF;

    QString text = "";
    if( m_printASCII ) text.append( byte );
    else               text = QString::number( byte )+" ";

    m_uartOutPanel.appendText( text );
}

#include "moc_terminalwidget.cpp"
