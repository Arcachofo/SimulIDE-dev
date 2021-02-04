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

#include "e-element.h"
#include "simulator.h"

eElement::eElement( QString id )
{
    m_elmId = id;

    added = false;

    digital_high = 5.0;
    digital_low  = 0.0;
    digital_thre = 2.5;

    if( Simulator::self() ) Simulator::self()->addToElementList( this );
    //qDebug() << "eElement::eElement" << QString::fromStdString( m_elmId );
    
}
eElement::~eElement()
{
    //qDebug() << "eElement::~eElement deleting" << QString::fromStdString( m_elmId );
    Simulator::self()->remFromElementList( this );
    Simulator::self()->cancelEvents( this );

    m_ePin.clear();
}

void eElement::setNumEpins( int n )
{
    m_ePin.resize(n);
    //qDebug() << "eElement::setNumEpins"<< QString::fromStdString( m_elmId )<<m_ePin.size();
    for( int i=0; i<n; i++ )
    {
        //qDebug() << "eElement::setNumEpins PIN:"<<i<<m_ePin[i];
        if( m_ePin[i] == 0 )
        {
            //qDebug() << "eElement::setNumEpins Creating:"<<i;
            m_ePin[i] = new ePin( m_elmId+"-ePin"+QString::number(i), i );
        }
    }
}

ePin* eElement::getEpin( int pin )
{
    return m_ePin[pin];
}

void eElement::setEpin( int num, ePin* pin )
{
    m_ePin[num] = pin;
}
