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

        QHBoxLayout* getLayout() { return mainLayout; }

    public slots:
        void on_expandButton_clicked();

    private:
        Oscope* m_oscope;
};


#endif
