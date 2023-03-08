/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

        enum action_t{
            actNone=0,
            actMove,
            actTime
        };

        PlotDisplay* display() { return plotDisplay; }
        QVBoxLayout* getDispLayout() { return dispLayout; }

        void setTrigger( int ch ) { triggerBox->setCurrentIndex( ch ); }

        void updateTimeDivBox( uint64_t timeDiv);
        void updateTimePosBox( int64_t timePos );
        void updateThresholdR( double v );
        void updateThresholdF( double v );

    public slots:
        void on_timeDivDial_valueChanged( int DialPos );
        void on_timeDivBox_valueChanged( double val );
        void on_timePosDial_valueChanged( int DialPos );
        void on_timePosBox_valueChanged( double val );

        void on_busGroup_buttonClicked( int ch );
        void setIsBus( int ch, bool b );

        void on_rThresholdBox_valueChanged( double v );
        void on_fThresholdBox_valueChanged( double v );

        void on_triggerBox_currentIndexChanged( int index );

        void on_condEdit_editingFinished();
        void setConds( QString conds ) { condEdit->setText( conds ); }

        void on_exportData_clicked();

    protected:
        void mousePressEvent( QMouseEvent* event );
        void mouseMoveEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event ) { m_action = actNone; setCursor( Qt::ArrowCursor ); }
        void closeEvent( QCloseEvent* event );
        void resizeEvent( QResizeEvent* ) { plotDisplay->updateValues(); }

        int m_timeDivDialPos;
        int m_timePosDialPos;
        int m_voltDivDialPos;

        int m_mousePos;
        action_t m_action;

        bool m_blocked;

        LAnalizer* m_analizer;
};


#endif
