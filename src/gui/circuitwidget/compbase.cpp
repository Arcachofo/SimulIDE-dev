/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "compbase.h"
#include "circuit.h"
#include "propdialog.h"
#include "comproperty.h"

CompBase::CompBase( QString type, QString id )
{
    m_id   = id;
    m_type = type;

    m_propDialog = nullptr;
}
CompBase::~CompBase()
{
    for( ComProperty* p : m_propMap.values() ) delete p;
    if( m_propDialog )
    {
        m_propDialog->setParent( nullptr );
        m_propDialog->close();
        delete m_propDialog;
    }
}

void CompBase::loadProperties( QVector<propStr_t> properties ) // Set properties in correct order
{
    for( propStr_t prop : properties ) // Set properties
        setPropStr( prop.name.toString(), prop.value.toString() );
}

void CompBase::remPropGroup( QString name )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        if( m_propGroups.at(i).name == name )
        {
            for( ComProperty* p : m_propGroups.at(i).propList ) m_propMap.remove( p->name() );
            m_propGroups.removeAt(i);
            break;
}   }   }

void CompBase::addPropGroup( propGroup pg, bool list )
{
    m_propGroups.append( pg );

    if( list )
        for( ComProperty* p : pg.propList ) m_propMap[p->name()] = p;
}

propGroup* CompBase::getPropGroup( QString name )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup& pg = m_propGroups[i];
        if( pg.name == name ) return &pg;
    }
    return nullptr;
}

void CompBase::addProperty( QString group, ComProperty* p, bool list )
{
    propGroup* pg = getPropGroup( group );
    if( !pg ) return;
    pg->propList.append( p );
    if( list ) m_propMap[p->name()] = p;
}

void CompBase::remProperty( QString prop )
{
    for( int i=0; i<m_propGroups.size(); ++i )
    {
        propGroup pg = m_propGroups.at(i);
        for( ComProperty* p : pg.propList )
        {
            if( p->name() != prop ) continue;
            pg.propList.removeAll( p );
            m_propGroups.replace( i, pg );
            m_propMap.remove( prop );
            delete p;
            return;
}   }   }

bool CompBase::setPropStr( QString prop, QString val )
{
    ComProperty* p = m_propMap.value( prop );
    if( !p ) return false;

    p->setValStr( val );
    if( m_propDialog ) m_propDialog->updtValues();

    return true;
}
QString CompBase::getPropStr( QString prop )
{
    ComProperty* p = m_propMap.value( prop );
    if( p ) return p->getValStr();
    return "";
}

QString CompBase::toString() // Used to save circuit
{
    QString item = "\n<item ";
    for( propGroup pg : m_propGroups )
    {
        if( !Circuit::self()->getBoard() )     // Not a Subcircit Board
        {
            if( pg.name == "Board") continue;  // Don't save Board properties
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
