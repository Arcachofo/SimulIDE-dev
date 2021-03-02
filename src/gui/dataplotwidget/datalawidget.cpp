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
 *                                                                         *
 ***************************************************************************/

#include "datalawidget.h"
#include "logicanalizer.h"
//#include "mainwindow.h"

DataLaWidget::DataLaWidget( QWidget* parent, LAnalizer* la )
          : QWidget( parent )
{
    m_analizer = la;

    setupUi(this);

    this->adjustSize();
    //fontScale->setValue( MainWindow::self()->fontScale() );
}

void DataLaWidget::on_expandButton_clicked()
{
    m_analizer->toggleExpand();
}

void DataLaWidget::setColor( int ch, QColor c )
{
    QString color = c.name();
    if( ch == 0 )
    {
        channel0->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 1 )
    {
        channel1->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 2 )
    {
        channel2->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 3 )
    {
        channel3->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 4 )
    {
        channel4->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 5 )
    {
        channel5->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 6 )
    {
        channel6->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 7 )
    {
        channel7->setStyleSheet( "background-color:"+color );
    }
}

