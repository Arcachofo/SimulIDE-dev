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

#ifndef LAWIDGET_H
#define LAWIDGET_H

#include <QDialog>
#include <QVector>

#include "ui_lawidget.h"
#include "plotdisplay.h"

class LAnalizer;

class MAINMODULE_EXPORT LaWidget : public QDialog, private Ui::LaWidget
{
        Q_OBJECT

    public:
        LaWidget( QWidget* parent, LAnalizer* la );
        ~LaWidget();

        PlotDisplay* display() { return plotDisplay; }
        QHBoxLayout* getLayout() { return mainLayout; }

        void setTrigger( int ch );

        void updateTimeDivBox( uint64_t timeDiv);
        void updateTimePosBox( int64_t timePos );
        void updateVoltDivBox( double voltDiv);

    public slots:
        void on_timeDivDial_valueChanged( int DialPos );
        void on_timeDivBox_valueChanged( double val );
        void on_timePosDial_valueChanged( int DialPos );
        void on_timePosBox_valueChanged( double val );

        void on_voltDivDial_valueChanged( int DialPos );
        void on_voltDivBox_valueChanged( double voltDiv );

        void on_triggerBox_currentIndexChanged( int index );

    protected:
        void closeEvent( QCloseEvent* event );
        void resizeEvent( QResizeEvent* event );

        int m_timeDivDialPos;
        int m_timePosDialPos;
        int m_voltDivDialPos;

        int m_channel;

        bool m_blocked;

        LAnalizer* m_analizer;
};


#endif
