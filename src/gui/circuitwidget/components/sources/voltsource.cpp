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

#include "voltsource.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

Component* VoltSource::construct( QObject* parent, QString type, QString id )
{
    return new VoltSource( parent, type, id );
}

LibraryItem* VoltSource::libraryItem()
{
    return new LibraryItem(
        tr( "Volt. Source" ),
        tr( "Sources" ),
        "voltsource.png",
        "Voltage Source",
        VoltSource::construct );
}

VoltSource::VoltSource( QObject* parent, QString type, QString id )
          : VarSource( parent, type, id )
{
    QString nodid = id;
    nodid.append("-outPin");
    nodid.append("-eSource");

    m_outPin = new IoPin( 0, QPoint(28,16), id+"-outPin", 0, this, source );
    m_pin.resize(1);
    m_pin[0] = m_outPin;

    m_outPin->setVoltHigh( 0 );
    m_outPin->setVoltLow( 0 );

    m_unit = "V";
    m_button->setText( QString("-- V") );
    setValue(5.0);
    valueChanged( 0 );
}
VoltSource::~VoltSource() {}

QList<propGroup_t> VoltSource::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Value", tr("Voltage"),"main"} );
    return {mainGroup};
}

void VoltSource::updateStep()
{
    if( m_changed ) 
    {
        m_outPin->setVoltHigh( m_outValue );
        m_outPin->setState( m_button->isChecked(), true );

        m_changed = false;
    }
}

#include "moc_voltsource.cpp"
