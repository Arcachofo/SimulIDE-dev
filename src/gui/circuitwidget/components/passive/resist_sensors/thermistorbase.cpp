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

#include "thermistorbase.h"

#include "doubleprop.h"

ThermistorBase::ThermistorBase( QObject* parent, QString type, QString id )
              : VarResBase( parent, type, id  )
{
    setValLabelPos(-16, 30, 0);

    m_ptc  = false;

    setMaxVal( 200 );
    setVal( 25 );

    addPropGroup( { tr("Main"), {
new DoubProp<ThermistorBase>( "Temp"     , tr("Current Value"),"ºC", this, &ThermistorBase::getVal,  &ThermistorBase::setVal ),
new DoubProp<ThermistorBase>( "Min_Temp" , tr("Minimum Value"),"ºC", this, &ThermistorBase::minVal,  &ThermistorBase::setMinVal ),
new DoubProp<ThermistorBase>( "Max_Temp" , tr("Maximum Value"),"ºC", this, &ThermistorBase::maxVal,  &ThermistorBase::setMaxVal ),
new DoubProp<ThermistorBase>( "Dial_Step", tr("Dial Step")    ,"ºC", this, &ThermistorBase::getStep, &ThermistorBase::setStep )
    }} );
}
ThermistorBase::~ThermistorBase(){}

void ThermistorBase::setPtc( bool ptc )
{
    return; // TODO
    m_ptc = ptc;
    m_changed = true;
}

