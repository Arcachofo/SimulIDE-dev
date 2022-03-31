/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include "module.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "tunnel.h"

#include "doubleprop.h"

ModuleSubc::ModuleSubc( QObject* parent, QString type, QString id )
          : ShieldSubc( parent, type, id )
{
    m_subcType = Chip::Module;
    setZValue( 2 );

    addPropGroup( { tr("Main"), {
new DoubProp<ModuleSubc>( "Z_Value", tr("Z Value"),"", this, &ModuleSubc::zVal, &ModuleSubc::setZVal )
    }} );
}
ModuleSubc::~ModuleSubc(){}

void ModuleSubc::setZVal( double v )
{
    if     ( v < 2 ) v = 2;
    else if( v > 9 ) v = 9;
    setZValue( v );
}

void ModuleSubc::attachToBoard()
{
    m_boardPos = m_circPos - m_board->pos();
}

void ModuleSubc::renameTunnels()
{
    for( Tunnel* tunnel : getPinTunnels() ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
    for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->tunnelUid() );
}
#include "moc_module.cpp"
