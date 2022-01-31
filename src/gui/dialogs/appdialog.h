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

#include <QDialog>

#include "ui_appdialog.h"

#ifndef APPDIALOG_H
#define APPDIALOG_H

class Component;
class PropVal;

class AppDialog : public QDialog, private Ui::AppDialog
{
    Q_OBJECT
    
    public:
        AppDialog( QWidget* parent=0 );

        void updtValues();

    public slots:
        void on_tabList_currentChanged( int );
//        void on_helpButton_clicked();

        // App Settings
        void on_language_currentIndexChanged( int index );
        void on_fontScale_valueChanged( double scale );

        // Circuit Settings
        void on_drawGrid_toggled( bool draw );
        void on_showScroll_toggled( bool show );
        void on_animate_toggled( bool ani );
        void on_fps_valueChanged( int fps );
        void on_backup_valueChanged( int secs );

        // Simulation Settings
        void on_simSpeedPerSlider_valueChanged( int speed );
        void on_simSpeedPerBox_editingFinished();
        void on_simSpeedSpsBox_editingFinished();

        void on_simStepUnitBox_currentIndexChanged( int index );
        void on_simStepBox_editingFinished();

        void on_nlStepsBox_editingFinished();

        void on_slopeStepsBox_editingFinished();

    private:
        void updateSpeed();
        void updateSpeedPer();

        bool m_blocked;

        double m_speedP;

        int      m_stepUnit;
        uint64_t m_step;
        uint64_t m_sps;
        uint64_t m_stepMult;
        uint64_t m_stepSize;

        bool m_helpExpanded;
};

#endif
