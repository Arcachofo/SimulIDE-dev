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

DataWidget::DataWidget( QWidget* parent, Oscope* oscope )
          : QWidget( parent )
{
    m_oscope = oscope;

    setupUi(this);

    m_chNames.append( channel0 );
    m_chNames.append( channel1 );
    m_chNames.append( channel2 );
    m_chNames.append( channel3 );

    QFont font = freq0->font();
    font.setFamily("Ubuntu");
    font.setPixelSize( 9 );
    freq0->setFont( font );
    freq1->setFont( font );
    freq2->setFont( font );
    freq3->setFont( font );
    expandButton->setFont( font );

    for( int i=0; i<4; ++i ) setData( i, "0.000 Hz" );

    this->adjustSize();
}

void DataWidget::on_expandButton_clicked()
{ m_oscope->toggleExpand(); }

void DataWidget::setColor( int ch, QColor c )
{
    QString color = c.name();
    m_chNames.at( ch )->setStyleSheet( "background-color:"+color );
}

void DataWidget::setData( int ch, QString freq )
{
    switch( ch )
    {
    case 0: freq0->setText( freq ); break;
    case 1: freq1->setText( freq ); break;
    case 2: freq2->setText( freq ); break;
    case 3: freq3->setText( freq );
}   }

void DataWidget::setTunnel( int ch, QString name )
{ m_chNames.at( ch )->setText( name ); }

void DataWidget::on_channel0_editingFinished()
{ m_oscope->channelChanged( 0 , channel0->text() ); }

void DataWidget::on_channel1_editingFinished()
{ m_oscope->channelChanged( 1 , channel1->text() ); }

void DataWidget::on_channel2_editingFinished()
{ m_oscope->channelChanged( 2 , channel2->text() ); }

void DataWidget::on_channel3_editingFinished()
{ m_oscope->channelChanged( 3 , channel3->text() ); }
