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


#include "piccomponent.h"
#include "piccomponentpin.h"
#include "mainwindow.h"
#include "circuit.h"
#include "utils.h"

LibraryItem* PICComponent::libraryItem()
{
    return new LibraryItem(
        "PIC",
        tr("Micro"),
        "ic2.png",
        "PIC",
    PICComponent::construct );
}

Component* PICComponent::construct( QObject* parent, QString type, QString id )
{
    PICComponent* pic = new PICComponent( parent, type,  id );
    if( m_error > 0 )
    {
        Circuit::self()->removeComp( pic );
        pic = 0l;
        m_error = 0;
        m_pSelf = 0l;
    }
    return pic;
}

PICComponent::PICComponent( QObject* parent, QString type, QString id )
            : McuComponent( parent, type, id )
            , m_pic( this )
{
    m_processor = &m_pic;
    m_cpi = 4;

    initChip();
    if( m_error == 0 )
    {
        setFreq( 20 );
        qDebug() <<"     ..."<<m_id<<"OK\n";
    }
    else qDebug() <<"     ..."<<m_id<<"Error!!!\n";
}
PICComponent::~PICComponent() { }

void PICComponent::attachPins()
{
    pic_processor* cpu = m_pic.getCpu();

    for( int i=0; i<m_numpins; i++ )
    {
        PICComponentPin* pin = dynamic_cast<PICComponentPin*>( m_pinList[i] );
        pin->attachPin( cpu );
    }
    m_attached = true;
}

void PICComponent::addPin( QString id, QString type, QString label,
                          int pos, int xpos, int ypos, int angle, int length )
{
    if( m_initialized )
    {
        updatePin( id, type, label, pos, xpos, ypos, angle );
    }
    else
    {
        m_pinList.append( new PICComponentPin( this, id, type, label, pos, xpos, ypos, angle ) );
    }
}

#include "moc_piccomponent.cpp"


