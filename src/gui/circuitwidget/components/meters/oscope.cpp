/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "oscope.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "oscopechannel.h"
//#include "pin.h"

static const char* Oscope_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Filter")
};

Component* Oscope::construct( QObject* parent, QString type, QString id )
{
    return new Oscope( parent, type, id );
}

LibraryItem* Oscope::libraryItem()
{
    return new LibraryItem(
        tr( "Oscope" ),
        tr( "Meters" ),
        "oscope.png",
        "Oscope",
        Oscope::construct );
}

Oscope::Oscope( QObject* parent, QString type, QString id )
      : PlotBase( parent, type, id )
{
    Q_UNUSED( Oscope_properties );

    m_graphical = true;

    setLabelPos(-80,-80, 0);

    m_baSizeX = 120;
    m_baSizeY = 100;
    m_adSizeX = 240;
    m_adSizeY = 200;

    //m_dataSize = 200;
    m_bufferSize = 600000;

    for(int i=0; i<2; i++ )
    {
        m_channel[i] = new OscopeChannel( (id+"Chan"+QString::number(i)));
        m_channel[i]->m_channel = i;
        m_channel[i]->m_ePin[0] = m_pin[i];
        m_channel[i]->m_ePin[1] = m_pin[2]; // Ref Pin
    }

    setAdvanc( false ); // Create Widgets
    m_filter = 0.0;
}
Oscope::~Oscope() {}

void Oscope::setFilter( double filter )
{
    m_filter = filter;

    for(int i=0; i<2; i++ ) m_channel[i]->setFilter( filter );
}

#include "moc_oscope.cpp"
