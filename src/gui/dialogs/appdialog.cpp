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

#include <math.h>

#include "appdialog.h"
#include "mainwindow.h"
#include "simulator.h"
#include "circuit.h"

AppDialog::AppDialog( QWidget* parent )
         : QDialog( parent )
{
    setupUi( this );
    //this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    // App Settings
    language->setCurrentIndex( (int)MainWindow::self()->lang() );
    fontScale->setValue( MainWindow::self()->fontScale() );

    // Circuit Settings
    drawGrid->setChecked( Circuit::self()->drawGrid() );
    showScroll->setChecked( Circuit::self()->showScroll() );
    animate->setChecked( Circuit::self()->animate() );
    fps->setValue( Simulator::self()->fps() );
    backup->setValue( Circuit::self()->autoBck() );

    // Simulation Settings
    m_blocked = true;

    m_stepSize = Simulator::self()->stepSize();
    m_sps      = Simulator::self()->stepsPerSec();

    m_speedP = 100*(double)m_sps*m_stepSize/1e12;
    simSpeedPerSlider->setValue( m_speedP );
    simSpeedPerBox->setValue( m_speedP );
    simSpeedSpsBox->setValue( m_sps );

    m_step = m_stepSize;
    m_stepMult = 1;
    m_stepUnit = 0;

    while( m_step > 999 )
    {
        m_step /= 1e3;
        m_stepMult *= 1e3;
        m_stepUnit += 1;
    }
    simStepBox->setValue( m_step );
    simStepUnitBox->setCurrentIndex( m_stepUnit );

    nlStepsBox->setValue( Simulator::self()->maxNlSteps() );
    slopeStepsBox->setValue( Simulator::self()->slopeSteps() );
    m_blocked = false;
    updateSpeedPer();

    m_helpExpanded = false;
    helpText->setVisible( false );
    mainLayout->removeWidget( helpText );
    //helpText->setText( help );
    this->adjustSize();
}

void AppDialog::on_tabList_currentChanged( int )
{
    if( !helpText->isVisible() )
    {
        QWidget* widget = tabList->currentWidget();
        this->setMaximumHeight( widget->minimumSizeHint().height()+150 );
    }
    this->adjustSize();
    this->setMaximumHeight( 800 );
}

/*void AppDialog::on_helpButton_clicked()
{
    m_helpExpanded = !m_helpExpanded;
    if( m_helpExpanded ) mainLayout->addWidget( helpText );
    else                 mainLayout->removeWidget( helpText );
    helpText->setVisible( m_helpExpanded );

    this->adjustSize();
}*/

void AppDialog::updtValues()
{

}

// App Settings -------------------------------

void AppDialog::on_language_currentIndexChanged( int index )
{
    MainWindow::self()->setLang( (Langs)index );
}

void AppDialog::on_fontScale_valueChanged( double scale )
{
    MainWindow::self()->setFontScale( scale );
}

// Circuit Settings ----------------------------

void AppDialog::on_drawGrid_toggled( bool draw )
{
    Circuit::self()->setDrawGrid( draw );
}
void AppDialog::on_showScroll_toggled( bool show )
{
    Circuit::self()->setShowScroll( show );
}

void AppDialog::on_animate_toggled( bool ani )
{
    Circuit::self()->setAnimate( ani );
}

void AppDialog::on_fps_valueChanged( int fps )
{
    Simulator::self()->setFps( fps );
}

void AppDialog::on_backup_valueChanged( int secs )
{
    Circuit::self()->setAutoBck( secs );
}

// Simulation Settings ----------------------
void AppDialog::on_simSpeedPerSlider_valueChanged( int speed )
{
    if( m_blocked ) return;
    m_speedP = speed;
    m_sps    = m_speedP*1e12/(m_stepSize*100);
    updateSpeed();
}

void AppDialog::on_simSpeedPerBox_editingFinished()
{
    m_blocked = true;
    uint64_t speedP = simSpeedPerBox->value();
    uint64_t sps = m_speedP*1e12/(m_stepSize*100);
    if( sps > 0 && speedP > 0 ){
        m_speedP = speedP;
        m_sps    = sps;
    }
    updateSpeed();
    m_blocked = false;
}

void AppDialog::on_simSpeedSpsBox_editingFinished()
{
    m_sps = simSpeedSpsBox->value();
    updateSpeedPer();
}

void AppDialog::on_simStepBox_editingFinished()
{
    m_step     = simStepBox->value();
    m_stepSize = m_step * m_stepMult;
    updateSpeedPer();
}

void AppDialog::on_simStepUnitBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;
    m_stepUnit = index;
    m_stepMult = pow( 1000, index );
    m_stepSize = m_step * m_stepMult;
    updateSpeedPer();
}

void AppDialog::updateSpeed()
{
    if( m_blocked ) return;
    m_blocked = true;

    simSpeedPerSlider->setValue( m_speedP );
    simSpeedPerBox->setValue( m_speedP );
    simSpeedSpsBox->setValue( m_sps );

    Simulator::self()->setStepSize( m_stepSize );
    Simulator::self()->setStepsPerSec( m_sps );
    m_blocked = false;
}

void AppDialog::updateSpeedPer()
{
    m_speedP = (double)(100*m_sps*m_stepSize)/1e12;
    if( m_speedP > 100 )
    {
        m_speedP = 100;
        m_sps = 1e12/m_stepSize;
    }
    updateSpeed();
}

void AppDialog::on_nlStepsBox_editingFinished()
{
    Simulator::self()->setMaxNlSteps( nlStepsBox->value() );
}

void AppDialog::on_slopeStepsBox_editingFinished()
{
    Simulator::self()->setSlopeSteps( slopeStepsBox->value() );
}
