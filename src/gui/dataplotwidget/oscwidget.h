/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        enum action_t{
            actNone=0,
            actMove,
            actTime
        };

        PlotDisplay* display() { return plotDisplay; }
        QHBoxLayout* getLayout() { return mainLayout; }

        void setFilter( double filter );
        void setAuto( int ch );
        void setTrigger( int ch );
        void hideChannel( int ch, bool hide );
        void setTracks( int tr ) { trackGroup->button( tr )->setChecked( true ); }

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

        void on_filterBox_valueChanged( double filter );

        void on_channelGroup_buttonClicked( int ch );
        void on_autoGroup_buttonClicked( int ch );
        void on_triggerGroup_buttonClicked( int ch );
        void on_hideGroup_buttonClicked( int ch );
        void on_trackGroup_buttonClicked( int ch );

    protected:
        void closeEvent( QCloseEvent* event );
        void resizeEvent( QResizeEvent* ) { plotDisplay->updateValues(); }
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event ) { m_action = actNone; setCursor( Qt::ArrowCursor ); }

        int m_timeDivDialPos;
        int m_timePosDialPos;
        int m_voltDivDialPos;
        int m_voltPosDialPos;

        int m_channel;
        int m_mousePos;
        action_t m_action;

        bool m_blocked;

        Oscope* m_oscope;
};


#endif
