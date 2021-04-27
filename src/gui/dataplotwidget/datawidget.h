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

#include <QWidget>

#include "ui_datawidget.h"

#ifndef DATAWIDGET_H
#define DATAWIDGET_H

class Oscope;

class DataWidget : public QWidget, private Ui::DataWidget
{
    Q_OBJECT

    public:
        DataWidget(QWidget* parent=0 , Oscope* oscope=0) ;

        void setColor( int ch, QColor c );
        void setData( int ch, QString freq );
        void setTunnel( int ch, QString name );

        QHBoxLayout* getLayout() { return mainLayout; }

    public slots:
        void on_expandButton_clicked();
        void on_channel0_editingFinished();
        void on_channel1_editingFinished();
        void on_channel2_editingFinished();
        void on_channel3_editingFinished();

    private:
        Oscope* m_oscope;

        QList<QLineEdit*> m_chNames;
};


#endif
