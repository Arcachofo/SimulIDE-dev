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

#include "ui_datalawidget.h"

#ifndef DATALAWIDGET_H
#define DATALAWIDGET_H

class LAnalizer;

class DataLaWidget : public QWidget, private Ui::DataLaWidget
{
    Q_OBJECT
    
    public:
        DataLaWidget(QWidget* parent=0 , LAnalizer* la=0) ;

        void setColor( int ch, QColor c );

        QHBoxLayout* getLayout() { return mainLayout; }

    public slots:
        void on_expandButton_clicked();

    private:
        LAnalizer* m_analizer;
};


#endif
