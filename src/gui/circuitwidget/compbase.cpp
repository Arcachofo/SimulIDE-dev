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

    m_group = NULL;
    m_propDialog = NULL;
}
CompBase::~CompBase()
{
    for( ComProperty* p : m_propHash.values() ) delete p;
}

void CompBase::remPropGroup( QString name )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        if( m_propGroups.at(i).name == name )
        {
            for( ComProperty* p : m_propGroups.at(i).propList ) m_propHash.remove( p->name() );
            m_propGroups.removeAt(i);
            break;
}   }   }

void CompBase::addPropGroup( propGroup pg )
{
    for( ComProperty* p : pg.propList ) m_propHash[p->name()] = p;
    m_propGroups.append( pg );
}

void CompBase::addProperty( QString group, ComProperty* p )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup pg = m_propGroups.at(i);
        if( pg.name != group ) continue;

        pg.propList.append( p );
        m_propGroups.replace( i, pg );
        m_propHash[p->name()] = p;
        return;
}   }

void CompBase::removeProperty( QString group, QString prop )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup pg = m_propGroups.at(i);
        if( pg.name != group ) continue;
        for( ComProperty* p : pg.propList )
        {
            if( p->name() != prop ) continue;
            pg.propList.removeAll( p );
            m_propGroups.replace( i, pg );
            m_propHash.remove( prop );
            return;
}   }   }

bool CompBase::setPropStr( QString prop, QString val )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ) p->setValStr( val );
    else return false;
    return true;
}
QString CompBase::getPropStr( QString prop )
{
    ComProperty* p = m_propHash.value( prop );
    if( p ) return p->getValStr();
    return "";
}

QString CompBase::toString() // Used to save circuit
{
    QString item = "\n<item ";
    for( propGroup pg : m_propGroups )
    {
        if( !m_saveBoard )
        {
            if( pg.name == "Board") continue;
            /// Error saving if package type is not set to board and have Main comp.
            /*if( m_isMainComp )
            {
                if( pg.name != "Main"
                 && pg.name != "Hidden" ) continue;
            }*/
        }
        for( ComProperty* prop : pg.propList )
        {
            QString val = prop->toString();
            if( val.isEmpty() ) continue;
            item += prop->name() + "=\""+val+"\" ";
    }   }
    item += "/>\n";

    return item;
}

int CompBase::getEnumIndex( QString prop )
{
    bool ok = false;
    int index = prop.toInt(&ok); // OLd TODELETE
    if( !ok ) index = m_enumUids.indexOf( prop );
    if( index < 0 || index > m_enumUids.size()-1) index = 0;
    return index;
}
