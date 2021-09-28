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

#include <QDomDocument>

#include "compbase.h"
#include "comproperty.h"

bool CompBase::m_saveBoard = false;

CompBase::CompBase( QObject* parent, QString type, QString id )
         : QObject( parent )
{
    m_id   = id;
    m_type = type;

    m_propDialog = NULL;
    m_graphical  = false;
}
CompBase::~CompBase()
{
    for( ComProperty* p : m_propHash.values() ) delete p;
}

void CompBase::remPropGroup( QString name )
{
    for( int i=0; i<m_properties.size(); ++i )
    {
        if( m_properties.at(i).name == name )
        {
            m_properties.removeAt(i);
            break;
}   }   }

void CompBase::addPropGroup( propGroup pg )
{
    for( ComProperty* p : pg.propList ) m_propHash[p->name()] = p;
    m_properties.append( pg );
}

bool CompBase::setProperty( QString prop, QString val )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ) p->setValStr( val );
    else return false;
    return true;
}
QString CompBase::getProperty( QString prop )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ) return p->getValStr();
    return "";
}

QString CompBase::toString() // Used to save circuit
{
    QString item = "\n<item ";
    for( propGroup pg : m_properties )
    {
        if( (pg.name == "Board") && !m_saveBoard ) continue;
        for( ComProperty* prop : pg.propList )
        {
            QString val = prop->getValStr();
            if( val.isEmpty() ) continue;
            item += prop->name() + "=\""+val+"\" ";
    }   }
    item += "/>\n";

    return item;
}
