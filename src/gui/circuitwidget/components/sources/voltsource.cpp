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

#include <QPushButton>

#include "voltsource.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "iopin.h"

#include "doubleprop.h"

Component* VoltSource::construct( QObject* parent, QString type, QString id )
{ return new VoltSource( parent, type, id ); }

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
    m_pin.resize(1);
    m_pin[0] = m_outPin = new IoPin( 0, QPoint(28,16), id+"-outPin", 0, this, source );

    m_outPin->setOutHighV( 0 );
    m_outPin->setOutLowV( 0 );

    m_unit = "V";
    addPropGroup( { tr("Main"), {
new DoubProp<VoltSource>( "MaxValue",tr("Max. Voltage"),"V", this, &VoltSource::maxValue,  &VoltSource::setMaxValue )
    }} );

    setShowProp("MaxValue");
    setPropStr( "MaxValue", "5 V" );
    valueChanged( 0 );
}
VoltSource::~VoltSource() {}

void VoltSource::updateStep()
{
    if( m_changed ) 
    {
        m_outPin->setOutHighV( m_outValue );
        m_outPin->setOutState( m_button->isChecked() );
        m_changed = false;
}   }

#include "moc_voltsource.cpp"
