/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "avrcomponent.h"
#include "circuit.h"
#include "utils.h"


LibraryItem* AVRComponent::libraryItem()
{
    return new LibraryItem(
        tr("AVR"),
        tr("Micro"),
        "ic2.png",
        "AVR",
        AVRComponent::construct );
}

Component* AVRComponent::construct( QObject* parent, QString type, QString id )
{
    AVRComponent* avr = new AVRComponent( parent, type,  id );
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( avr );
        avr->deleteLater();
        avr = 0l;
        m_error = 0;
        m_pSelf = 0l;
    }
    return avr;
}

AVRComponent::AVRComponent( QObject* parent, QString type, QString id )
            : AvrCompBase( parent, type, id )
{
    initChip();
    if( m_error == 0 )
    {
        setFreq( 16 );
        qDebug() <<"     ..."<<m_id<<"OK\n";
    }
    else
    {
        qDebug() <<"     ..."<<m_id<<"Error!!!\n";
    }
}
AVRComponent::~AVRComponent() { }


#include "moc_avrcomponent.cpp"
