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

#include "datawidget.h"
#include "oscope.h"
//#include "mainwindow.h"

DataWidget::DataWidget( QWidget* parent, Oscope* oscope )
          : QWidget( parent )
{
    m_oscope = oscope;

    setupUi(this);

    for( int i=0; i<4; ++i ) setData( i, "0.0000 Hz" );

    this->adjustSize();
    //fontScale->setValue( MainWindow::self()->fontScale() );
}

void DataWidget::on_expandButton_clicked()
{
    m_oscope->toggleExpand();
}

void DataWidget::setColor( int ch, QColor c )
{
    QString color = c.name();
    if( ch == 0 )
    {
        freq0->setStyleSheet( "background-color:"+color );
        channel0->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 1 )
    {
        freq1->setStyleSheet( "background-color:"+color );
        channel1->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 2 )
    {
        freq2->setStyleSheet( "background-color:"+color );
        channel2->setStyleSheet( "background-color:"+color );
    }
    else if( ch == 3 )
    {
        freq3->setStyleSheet( "background-color:"+color );
        channel3->setStyleSheet( "background-color:"+color );
    }
}

void DataWidget::setData( int ch, QString freq )
{
    if( ch == 0 )
    {
        freq0->setText( freq );
    }
    else if( ch == 1 )
    {
        freq1->setText( freq );
    }
    else if( ch == 2 )
    {
        freq2->setText( freq );
    }
    else if( ch == 3 )
    {
        freq3->setText( freq );
    }
}
