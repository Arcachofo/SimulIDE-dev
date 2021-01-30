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

#ifndef OSCWIDGET_H
#define OSCWIDGET_H

#include <QDialog>

#include "ui_oscwidget.h"
#include "plotdisplay.h"

class Oscope;

class MAINMODULE_EXPORT OscWidget : public QDialog, private Ui::OscWidget
{
        Q_OBJECT

    public:
        OscWidget( QWidget* parent , Oscope* oscope );
        ~OscWidget();

        PlotDisplay* display() { return plotDisplay; }
        QHBoxLayout* getLayout() { return mainLayout; }

        void setAuto( int ch );
        void setTrigger( int ch );
        void hideChannel( int ch, bool hide );
        void setTracks( int tr );

        void updateTimeDivBox( uint64_t timeDiv);
        void updateTimePosBox( int ch, int64_t timePos );
        void updateVoltDivBox( int ch, double voltDiv);
        void updateVoltPosBox( int ch, double voltPos );

    public slots:
        void on_timeDivDial_valueChanged( int DialPos );
        void on_timeDivBox_valueChanged( double val );
        void on_timePosDial_valueChanged( int DialPos );
        void on_timePosBox_valueChanged( double val );

        void on_voltDivDial_valueChanged( int DialPos );
        void on_voltDivBox_valueChanged( double voltDiv );
        void on_voltPosDial_valueChanged( int DialPos );
        void on_voltPosBox_valueChanged( double voltPos );

        void on_channelGroup_buttonClicked( int ch );
        void on_autoGroup_buttonClicked( int ch );
        void on_triggerGroup_buttonClicked( int ch );
        void on_hideGroup_buttonClicked( int ch );
        void on_trackGroup_buttonClicked( int ch );

    protected:
        void closeEvent( QCloseEvent* event );
        void resizeEvent( QResizeEvent* event );

        int m_timeDivDialPos;
        int m_timePosDialPos;
        int m_voltDivDialPos;
        int m_voltPosDialPos;

        int m_channel;

        bool m_blocked;

        Oscope* m_oscope;
};


#endif
