/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         */

#include <QWidget>

#include "ui_datalawidget.h"

#ifndef DATALAWIDGET_H
#define DATALAWIDGET_H

class PlotBase;

class DataLaWidget : public QWidget, private Ui::DataLaWidget
{
    Q_OBJECT
    
    public:
        DataLaWidget(QWidget* parent=0 , PlotBase* pb=0) ;

        void setColor( int ch, QColor c );
        void setTunnel( int ch, QString name );

        QHBoxLayout* getLayout() { return mainLayout; }

    public slots:
        void on_expandButton_clicked();
        void on_channel0_editingFinished();
        void on_channel1_editingFinished();
        void on_channel2_editingFinished();
        void on_channel3_editingFinished();
        void on_channel4_editingFinished();
        void on_channel5_editingFinished();
        void on_channel6_editingFinished();
        void on_channel7_editingFinished();

    private:
        PlotBase* m_plotBase;

        QList<QLineEdit*> m_chNames;
};


#endif
