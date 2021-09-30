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

#include "capacitorbase.h"
#include "pin.h"

#include "doubleprop.h"

CapacitorBase::CapacitorBase( QObject* parent, QString type, QString id )
             : Comp2Pin( parent, type, id )
             , eCapacitor( id )
{
    m_area = QRectF( -10, -10, 20, 20 );

    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];
    m_pin[0]->setLength( 12 );
    m_pin[1]->setLength( 12 );
    
    setShowProp("Capacitance");

    addPropGroup( { tr("Main"), {
new DoubProp<CapacitorBase>( "Capacitance", tr("Capacitance"), "F", this, &CapacitorBase::cap, &CapacitorBase::setCap )
    } } );
}
CapacitorBase::~CapacitorBase(){}

