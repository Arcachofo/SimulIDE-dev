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
#include "itemlibrary.h"
#include "pin.h"

#include "stringprop.h"
#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

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
    addPropGroup( { tr("Main"), {
new BoolProp  <RelaySPST>( "Norm_Close", tr("Normally Closed"),"", this, &RelaySPST::nClose, &RelaySPST::setNClose ),
new BoolProp  <RelaySPST>( "DT"        , tr("Double Throw")   ,"", this, &RelaySPST::dt,     &RelaySPST::setDt ),
new IntProp   <RelaySPST>( "Poles"     , tr("Poles")          ,"_Poles", this, &RelaySPST::poles,  &RelaySPST::setPoles, "uint" ),
//new StringProp<RelaySPST>( "Key"       , tr("Key")            ,"", this, &RelaySPST::key,    &RelaySPST::setKey ),
    }} );
    addPropGroup( { tr("Electric"), {
new DoubProp<RelaySPST>( "Rcoil"     , tr("Resistance"),"Ω", this, &RelaySPST::rCoil, &RelaySPST::setRCoil),
new DoubProp<RelaySPST>( "Inductance", tr("Inductance"),"H", this, &RelaySPST::induc, &RelaySPST::setInduc ),
new DoubProp<RelaySPST>( "IOn"       , tr("IOn")       ,"A", this, &RelaySPST::iTrig, &RelaySPST::setITrig),
new DoubProp<RelaySPST>( "IOff"      , tr("IOff")      ,"A", this, &RelaySPST::iRel,  &RelaySPST::setIRel ),
    }} );
}
RelaySPST::~RelaySPST(){}

