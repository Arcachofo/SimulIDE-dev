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

#include "relay-spst.h"
#include "pin.h"

Component* RelaySPST::construct( QObject* parent, QString type, QString id )
{ return new RelaySPST( parent, type, id ); }

LibraryItem* RelaySPST::libraryItem()
{
    return new LibraryItem(
            tr( "Relay (all)" ),
            tr( "Switches" ),
            "relay-spst.png",
            "RelaySPST",
            RelaySPST::construct);
}

RelaySPST::RelaySPST( QObject* parent, QString type, QString id )
         : RelayBase( parent, type, id )
{
    //m_area = QRectF( -10, -26, 20, 36 );
}
RelaySPST::~RelaySPST(){}

QList<propGroup_t> RelaySPST::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Norm_Close", tr("Normally Closed"),""} );
    mainGroup.propList.append( {"DT", tr("Double Throw"),""} );
    mainGroup.propList.append( {"Poles", tr("Poles"),""} );
    mainGroup.propList.append( {"Key", tr("Key"),""} );

    propGroup_t elecGroup { tr("Electric") };
    elecGroup.propList.append( {"Rcoil", tr("Resistance"),"Ω"} );
    elecGroup.propList.append( {"Inductance", tr("Inductance"),"H"} );
    elecGroup.propList.append( {"IOn", tr("IOn"),"A"} );
    elecGroup.propList.append( {"IOff", tr("IOff"),"A"} );

    return {mainGroup, elecGroup};
}

#include "moc_relay-spst.cpp"
