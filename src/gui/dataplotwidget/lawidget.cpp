/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMouseEvent>

#include "lawidget.h"
#include "logicanalizer.h"
#include "utils.h"
#include "mainwindow.h"

LaWidget::LaWidget( QWidget* parent , LAnalizer* la )
        : QDialog( parent )
{
    setupUi(this);

    m_analizer = la;
    m_blocked = false;
    m_action = actNone;

    /*double fontScale = MainWindow::self()->fontScale();
    QFont font = timeDivLabel->font();
    font.setFamily("Ubuntu");
    font.setBold( true );

    font.setPixelSize( 14*fontScale );
    timeDivLabel->setFont( font );
    timePosLabel->setFont( font );
    voltDivLabel->setFont( font );
    trigLabel->setFont( font );

    font.setPixelSize( 10*fontScale );
    timeDivBox->setFont( font );
    timePosBox->setFont( font );
    voltDivBox->setFont( font );
    triggerBox->setFont( font );
    condEdit->setFont( font );*/
}
LaWidget::~LaWidget(){}

void LaWidget::on_timeDivDial_valueChanged( int DialPos )
{
    uint64_t timeDiv = m_analizer->timeDiv();
    uint64_t   delta = timeDiv/100;
    if( delta < 1 ) delta = 1;

    if( DialPos < m_timeDivDialPos ) timeDiv += delta;
    else                             timeDiv -= delta;

    m_analizer->setTimeDiv( timeDiv );
    m_timeDivDialPos = DialPos;
}

void LaWidget::on_timeDivBox_valueChanged( double timeDiv ) // User entered value
{
    if( m_blocked ) return;

    QString unit = timeDivBox->suffix().remove("s");
    unitToVal( timeDiv, unit );
    m_analizer->setTimeDiv( timeDiv );
}

void LaWidget::updateTimeDivBox( uint64_t timeDiv )
{
    m_blocked = true;
    double val = timeDiv;
    QString unit = " p";
    int Vdecimals = 0;
    valToUnit( val, unit, Vdecimals );

    timeDivBox->setDecimals( Vdecimals );
    timeDivBox->setValue( val );
    timeDivBox->setSuffix( unit+"s" );
    m_blocked = false;
}

void LaWidget::on_timePosDial_valueChanged( int DialPos )
{
    int64_t timePos = m_analizer->timePos();
    int64_t   delta = m_analizer->timeDiv()/100;
    if( delta < 1 ) delta = 1;

    if( DialPos < m_timePosDialPos ) timePos += delta;
    else                             timePos -= delta;

    m_analizer->setTimePos( timePos );
    m_timePosDialPos = DialPos;
}

void LaWidget::on_timePosBox_valueChanged( double timePos )
{
    if( m_blocked ) return;

    QString unit = timePosBox->suffix().remove("s");
    unitToVal( timePos, unit );
    m_analizer->setTimePos( timePos );
}

void LaWidget::updateTimePosBox( int64_t timePos )
{
    m_blocked = true;

    double val = timePos;
    QString unit = " p";
    int Vdecimals = 0;
    valToUnit( val, unit, Vdecimals )

    timePosBox->setDecimals( Vdecimals );
    timePosBox->setValue( val );
    timePosBox->setSuffix( unit+"s" );
    m_blocked = false;
}

void LaWidget::on_voltDivDial_valueChanged( int DialPos )
{
    double voltDiv = m_analizer->voltDiv();
    double delta = voltDiv/100;
    if( DialPos < m_voltDivDialPos ) voltDiv += delta;
    else                             voltDiv -= delta;

    m_analizer->setVoltDiv( voltDiv );
    m_voltDivDialPos = DialPos;
}

void LaWidget::on_voltDivBox_valueChanged( double voltDiv )
{
    if( m_blocked ) return;

    QString unit = voltDivBox->suffix().remove("V");
    unitToVal( voltDiv, unit );
    m_analizer->setVoltDiv( voltDiv/1e12 );
}

void LaWidget::updateVoltDivBox( double voltDiv )
{
    m_blocked = true;

    double val  = voltDiv*1e12;
    QString unit = " p";
    int Vdecimals = 0;
    valToUnit( val, unit, Vdecimals );

    voltDivBox->setDecimals( Vdecimals );
    voltDivBox->setValue( val );
    voltDivBox->setSuffix( unit+"V" );
    m_blocked = false;
}

void LaWidget::on_triggerBox_currentIndexChanged( int index )
{ m_analizer->setTrigger( index ); }


void LaWidget::on_condEdit_editingFinished()
{ m_analizer->updateConds( condEdit->text() ); }

void LaWidget::closeEvent( QCloseEvent* event )
{
    if( !parent() ) return;
    m_analizer->expand( false );
    QWidget::closeEvent( event );
}

void LaWidget::mousePressEvent( QMouseEvent* event )
{
    m_mousePos = event->globalX();
    if( event->button() == Qt::LeftButton )
    {
        m_action = actMove;
        setCursor( Qt::ClosedHandCursor );
    }
    else if( event->button() == Qt::MidButton )
    {
        m_action = actTime;
        plotDisplay->setTimeZero( m_mousePos );
    }
}

void LaWidget::mouseMoveEvent( QMouseEvent* event )
{
    int pos = event->globalX();

    if( m_action == actMove )
    {
        double timePos = m_analizer->timePos();
        double timeX = m_analizer->timeDiv()*10;
        double sizeX = plotDisplay->sizeX();
        double deltaT = (m_mousePos - pos)*timeX/sizeX;
        m_analizer->setTimePos( timePos+deltaT );
    }
    m_mousePos = pos;
}

