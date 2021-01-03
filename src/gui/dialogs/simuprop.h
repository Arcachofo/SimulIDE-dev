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
 *                                                                         */

#include <QDialog>

#include "ui_simuprop.h"

#ifndef SIMDIALOG_H
#define SIMDIALOG_H


class SimuProp : public QDialog, private Ui::SimuProp
{
    Q_OBJECT
    
    public:
        SimuProp( QWidget* parent=0 );

    public slots:
        void on_simSpeedPerSlider_valueChanged( int speed );
        void on_simSpeedPerBox_editingFinished();
        void on_simSpeedSpsBox_editingFinished();

        void on_simStepUnitBox_currentIndexChanged( int index );
        void on_simStepBox_editingFinished();

        void on_nlStepsBox_editingFinished();

        void on_helpButton_toggled();

    private:
        void updateSpeed();

        bool m_blocked;

        double m_speedP;

        int      m_stepUnit;
        uint64_t m_step;
        uint64_t m_sps;
        uint64_t m_stepMult;
        uint64_t m_stepSize;

};


#endif
