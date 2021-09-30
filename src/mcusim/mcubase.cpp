/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "mcubase.h"
#include "mcuinterface.h"

#include "stringprop.h"

McuBase* McuBase::m_pSelf = NULL;

McuBase::McuBase( QObject* parent, QString type, QString id )
       : Chip( parent, type, id )
{
    addPropGroup( { tr("Hidden"), {
new StringProp<McuBase>( "varList", "","", this, &McuBase::varList,   &McuBase::setVarList),
new StringProp<McuBase>( "eeprom" , "","", this, &McuBase::getEeprom, &McuBase::setEeprom )
    }} );
}
McuBase::~McuBase()
{
    if( m_pSelf == this ) m_pSelf= NULL;
}

bool McuBase::setProperty( QString prop, QString val )
{
    if( prop =="Mhz" ) setFreq( val.toDouble()*1e6 ); //  Old: TODELETE
    else return Component::setProperty( prop, val );
    return true;
}

QString McuBase::varList()
{
    return m_proc->getRamTable()->getVarSet().join(",");
}

void McuBase::setVarList( QString vl )
{
    m_proc->getRamTable()->loadVarSet( vl.split(",") );
}

void McuBase::setEeprom( QString eep )
{
    if( eep.isEmpty() ) return;
    QVector<int> eeprom;
    QStringList list = eep.split(",");
    for( QString val : list ) eeprom.append( val.toUInt() );

    if( eeprom.size() > 0 ) m_proc->setEeprom( &eeprom );
}

QString McuBase::getEeprom()  // Used by property, stripped to last written value.
{
    QString eeprom;
    int size = m_proc->romSize();
    if( size > 0 )
    {
        bool empty = true;
        for( int i=size-1; i>=0; --i )
        {
            uint8_t val = m_proc->getRomValue( i );
            if( val < 0xFF ) empty = false;
            if( empty ) continue;
            eeprom.prepend( QString::number( val )+"," );
    }   }
    return eeprom;
}

