/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDialog>

#include "ui_appdialog.h"

#ifndef APPDIALOG_H
#define APPDIALOG_H

class PropVal;

class AppDialog : public QDialog, private Ui::AppDialog
{
    Q_OBJECT
    
    public:
        AppDialog( QWidget* parent=0 );

        void updtValues();

    public slots:
        void on_tabList_currentChanged( int );
        void on_helpButton0_clicked() { m_showHelp = !m_showHelp;updtHelp(); }
        void on_helpButton1_clicked() { m_showHelp = !m_showHelp;updtHelp(); }
        void on_helpButton2_clicked() { m_showHelp = !m_showHelp;updtHelp(); }

        // App Settings
        void on_language_currentIndexChanged( int index );
        void on_fontScale_valueChanged( double scale );
        void on_setPathButton_clicked();
        void on_userPath_editingFinished();

        // Circuit Settings
        void on_drawGrid_toggled( bool draw );
        void on_showScroll_toggled( bool show );
        void on_animate_toggled( bool ani );
        void on_fps_valueChanged( int fps );
        void on_backup_valueChanged( int secs );

        // Simulation Settings
        void on_simSpeedPerSlider_valueChanged( int speed );

        void on_simStepUnitBox_currentIndexChanged( int index );
        void on_simStepBox_editingFinished();

        void on_nlStepsBox_editingFinished();

        void on_reactStepUnitBox_currentIndexChanged( int index );
        void on_reactStepBox_editingFinished();

        void on_slopeStepsBox_editingFinished();

    private:
        void updtHelp();
        void updtSpeed();
        void updtSpeedPer();
        void updtReactStep();

        bool m_blocked;
        bool m_showHelp;

        double m_speedP;

        int      m_stepUnit;
        uint64_t m_stepsPS;
        uint64_t m_stepSize;
        uint64_t m_psPerSec;
};

#endif
