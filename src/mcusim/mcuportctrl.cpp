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

#include <QDebug>

#include "mcuportctrl.h"
#include "iopin.h"

McuCtrlPort::McuCtrlPort( eMcu* mcu, QString name )
           : McuModule( mcu, name )
{
    //m_numPins = numPins;

}
McuCtrlPort::~McuCtrlPort(){}

void McuCtrlPort::createPins( Mcu* mcuComp, QString pins )
{
    QStringList pinList = pins.split(",");
    for( QString pinName : pinList )
    {
        IoPin* pin = new IoPin( 0, QPoint(0,0), m_name+pinName, 0, (Component*)mcuComp );
        m_pins.emplace_back( pin );
    }
}

/*IoPin* McuCtrlPort::getPinN( uint8_t i )
{
    if( i >= m_pins.size() ) return NULL;
    return m_pins[i];
}*/

IoPin* McuCtrlPort::getPin( QString pinId )
{
    IoPin* pin = NULL;
    for( IoPin* ioPin : m_pins )
        if( ioPin->pinId().contains( pinId ) ) pin = ioPin;

    if( !pin ) qDebug() << "ERROR: NULL Pin:"<< pinId;
    return pin;
}
