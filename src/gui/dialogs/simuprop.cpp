/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "simuprop.h"
#include "simulator.h"

SimuProp::SimuProp( QWidget* parent )
        : QDialog( parent )
{
    setupUi(this);

    m_blocked = false;

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

    helpText->setVisible( false );
    mainLayout->removeWidget( helpText );
    this->adjustSize();
}

void SimuProp::on_simSpeedPerSlider_valueChanged( int speed )
{
    if( m_blocked ) return;
    m_speedP = speed;
    m_sps    = m_speedP*1e12/(m_stepSize*100);
    updateSpeed();
}

void SimuProp::on_simSpeedPerBox_editingFinished()
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

void SimuProp::on_simSpeedSpsBox_editingFinished()
{
    uint64_t sps = simSpeedSpsBox->value();
    double speedP = (double)(100*sps*m_stepSize)/1e12;
    if( speedP > 100 )
    {
        m_speedP = 100;
        sps = 1e12/m_stepSize;
    }

    m_speedP = speedP;
    m_sps = sps;
    updateSpeed();
}

void SimuProp::on_simStepBox_editingFinished()
{
    m_step     = simStepBox->value();
    m_stepSize = m_step * m_stepMult;
    m_sps      = m_speedP*1e12/(m_stepSize*100);
    updateSpeed();
}

void SimuProp::on_simStepUnitBox_currentIndexChanged( int index )
{
    m_stepUnit = index;
    m_stepMult = pow( 1000, index );
    m_stepSize = m_step * m_stepMult;
    m_sps      = m_speedP*1e12/(m_stepSize*100);
    updateSpeed();
}

void SimuProp::updateSpeed()
{
    m_blocked = true;
    /*while( m_sps == 0 && m_step > 1 )
    {
        m_step -= 1;

        if( m_stepSize == 0 )
        {
            if( m_stepMult >= 1e3 )
            {
                m_step = 100;
                m_stepMult /= 1e3;
            }
            else m_stepSize = 1;
        }
        m_sps = m_speedP*1e12/(m_stepSize*100);
    }*/
    simSpeedPerSlider->setValue( m_speedP );
    simSpeedPerBox->setValue( m_speedP );
    simSpeedSpsBox->setValue( m_sps );

    Simulator::self()->setStepSize( m_stepSize );
    Simulator::self()->setStepsPerSec( m_sps );
    m_blocked = false;
}

void SimuProp::on_nlStepsBox_editingFinished()
{
    Simulator::self()->setMaxNlSteps( nlStepsBox->value() );
}

void SimuProp::on_helpButton_toggled()
{
    bool expanded = helpButton->isChecked();
    if( expanded ) mainLayout->addWidget(helpText);
    else           mainLayout->removeWidget(helpText);
    helpText->setVisible( expanded );
    this->adjustSize();
}

