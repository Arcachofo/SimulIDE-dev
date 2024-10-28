/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <math.h>

#include "appdialog.h"
#include "mainwindow.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"

AppDialog::AppDialog( QWidget* parent )
         : QDialog( parent )
{
    setupUi( this );
    //this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    // App Settings
    m_scale = MainWindow::self()->fontScale();
    language->setCurrentIndex( (int)MainWindow::self()->lang() );
    language->setFixedWidth( 80*m_scale );
    fontName->setCurrentText( MainWindow::self()->defaultFontName() );
    fontName->setFixedWidth( 80*m_scale );
    fontScale->setValue( m_scale );
    fontScale->setFixedWidth( 80*m_scale );
    userPath->setText( MainWindow::self()->userPath() );

    // Circuit Settings
    drawGrid->setChecked( Circuit::self()->drawGrid() );
    showScroll->setChecked( CircuitView::self()->showScroll() );
    animate->setChecked( Circuit::self()->animate() );
    canvasWidth->setValue( Circuit::self()->sceneWidth() );
    canvasHeight->setValue( Circuit::self()->sceneHeight() );
    fps->setValue( Simulator::self()->fps() );
    backup->setValue( Circuit::self()->autoBck() );
    undo_steps->setValue( Circuit::self()->undoSteps() );

    // Simulation Settings
    m_blocked = true;

    m_psPerSec = Simulator::self()->psPerSec();
    m_speedP   = 100*(double)m_psPerSec/1e12;
    m_stepsPS  = Simulator::self()->stepsPerSec();
    m_stepSize = Simulator::self()->stepSize();
    m_stepUnit = log10(m_stepSize)/3;

    uint64_t reactStep = Simulator::self()->reactStep();
    int unit = 0; // ps
    uint64_t mult = 1;
    uint64_t step = reactStep / mult;
    while( remainder( reactStep, mult*1e3 ) == 0 )
    {
        unit += 1;
        mult *= 1e3;
        step = reactStep / mult;
    }
    reactStepBox->setValue( step );
    reactStepUnitBox->setCurrentIndex( unit );

    nlStepsBox->setValue( Simulator::self()->maxNlSteps() );
    slopeStepsBox->setValue( Simulator::self()->slopeSteps() );
    m_blocked = false;

    updtSpeedPer();

    m_showHelp = false;
    helpText->setVisible( false );
    mainLayout->removeWidget( helpText );
    //helpText->setText( help );

    this->adjustSize();
}

void AppDialog::on_tabList_currentChanged( int tab )
{
    if( helpText->isVisible() ) updtHelp();

    switch( tab ) {
        case 0: setMinimumHeight( 250*m_scale ); setMaximumHeight( 350*m_scale ); break;
        case 1: setMinimumHeight( 300*m_scale ); setMaximumHeight( 400*m_scale ); break;
        case 2: setMinimumHeight( 350*m_scale ); setMaximumHeight( 450*m_scale ); break;
    }
    adjustSize();
}
void AppDialog::updtHelp()
{
    if( m_showHelp ){
        if( !helpText->isVisible() ) mainLayout->addWidget( helpText );

        QString tabStr;
        switch( tabList->currentIndex() ) {
            case 0: tabStr = "app";        break;
            case 1: tabStr = "circuit";    break;
            case 2: tabStr = "simulation"; break;
        }
        helpText->setText( MainWindow::self()->getHelp( tabStr) );
    }
    else mainLayout->removeWidget( helpText );

    helpText->setVisible( m_showHelp );
    adjustSize();
}

void AppDialog::updtValues(){}

// App Settings -------------------------------

void AppDialog::on_language_currentIndexChanged( int index )
{
    MainWindow::self()->setLang( (Langs)index );
}

void AppDialog::on_fontScale_valueChanged( double scale )
{
    MainWindow::self()->setFontScale( scale );
}

void AppDialog::on_setPathButton_clicked()
{
    MainWindow::self()->getUserPath();
    userPath->setText( MainWindow::self()->userPath() );
}

void AppDialog::on_userPath_editingFinished()
{
    QString path = userPath->text();
    MainWindow::self()->setUserPath( path );
}

// Circuit Settings ----------------------------

void AppDialog::on_drawGrid_toggled( bool draw )
{
    Circuit::self()->setDrawGrid( draw );
}
void AppDialog::on_showScroll_toggled( bool show )
{
    CircuitView::self()->setShowScroll( show );
}

void AppDialog::on_animate_toggled( bool ani )
{
    Circuit::self()->setAnimate( ani );
}

void AppDialog::on_canvasWidth_editingFinished()
{
    Circuit::self()->setSceneWidth( canvasWidth->value() );
}

void AppDialog::on_canvasHeight_editingFinished()
{
    Circuit::self()->setSceneHeight( canvasHeight->value() );
}

void AppDialog::on_fps_valueChanged( int fps )
{
    Simulator::self()->setFps( fps );
}

void AppDialog::on_backup_valueChanged( int secs )
{
    Circuit::self()->setAutoBck( secs );
}

void AppDialog::on_undo_steps_valueChanged( int steps )
{
    Circuit::self()->setUndoSteps( steps );
}

// Simulation Settings ----------------------
void AppDialog::on_simSpeedPerSlider_valueChanged( int speed )
{
    if( m_blocked ) return;
    if( speed == 0 ) speed = 1;
    m_speedP = speed;
    m_psPerSec = m_speedP*1e12/100;
    updtSpeed();
}

void AppDialog::on_simStepBox_editingFinished()
{
    m_stepsPS = simStepBox->value();
    m_psPerSec = m_stepsPS * m_stepSize;
    updtSpeedPer();
}

void AppDialog::on_simStepUnitBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;
    m_stepUnit = index;
    m_stepSize = pow( 1000, index );
    m_psPerSec = m_stepsPS * m_stepSize;
    updtSpeedPer();
}

void AppDialog::updtSpeed()
{
    if( m_blocked ) return;
    m_blocked = true;

    simSpeedPerSlider->setValue( m_speedP );
    speedLabel->setText( " "+QString::number( m_speedP,'f', 2 )+"%");

    m_stepsPS = m_psPerSec/m_stepSize;
    while( m_stepsPS >= 1000000000 )
    {
        m_stepsPS /= 1e3;
        m_stepSize *= 1e3;
        m_stepUnit += 1;
    }
    while( m_stepsPS == 0 )
    {
        m_stepSize /= 1e3;
        m_stepUnit -= 1;
        m_stepsPS = m_psPerSec/m_stepSize;
    }
    simStepBox->setValue( m_stepsPS );
    simStepUnitBox->setCurrentIndex( m_stepUnit );

    Simulator::self()->setStepSize( m_stepSize );
    Simulator::self()->setStepsPerSec( m_stepsPS );
    Simulator::self()->setPsPerSec( m_psPerSec );
    m_blocked = false;
}

void AppDialog::updtSpeedPer()
{
    m_speedP = (double)(100*m_psPerSec)/1e12;
    if( m_speedP > 100 )
    {
        m_speedP = 100;
        m_psPerSec = 1e12;
    }
    updtSpeed();
}

void AppDialog::on_nlStepsBox_editingFinished()
{
    Simulator::self()->setMaxNlSteps( nlStepsBox->value() );
}

void AppDialog::on_reactStepUnitBox_currentIndexChanged( int index )
{
    updtReactStep();
}

void AppDialog::on_reactStepBox_editingFinished()
{
    updtReactStep();
}

void AppDialog::updtReactStep()
{
    if( m_blocked ) return;
    uint64_t mult = pow( 1000, reactStepUnitBox->currentIndex() );
    uint64_t reactStep = mult*reactStepBox->value();
    Simulator::self()->setReactStep( reactStep );
}

void AppDialog::on_slopeStepsBox_editingFinished()
{
    Simulator::self()->setSlopeSteps( slopeStepsBox->value() );
}

void AppDialog::on_fontName_currentFontChanged( const QFont &f )
{
    MainWindow::self()->setDefaultFontName( f.family() );
}

