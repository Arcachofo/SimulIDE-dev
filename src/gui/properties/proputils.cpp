/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QStringList>
#include <QPointF>
#include "proputils.h"
//#include "utils.h"

QPointF getPointF( QString p )
{
    QStringList plist = p.split(",");
    QPointF point( plist.first().toDouble(), plist.last().toDouble() );
    return point;
}
QString getStrPointF( QPointF p )
{
    return QString::number( p.x())+","+QString::number(p.y() );
}

//---------------------------------------------------

QVector<propStr_t> parseProps( QStringRef line )
{
    QVector<propStr_t> properties;

    QStringRef name;
    QVector<QStringRef> tokens = line.split("\"");
    tokens.removeLast();

    for( QStringRef token : tokens )
    {
        if( token.endsWith("=") )
        {
            int start = token.lastIndexOf(" ")+1;
            name = token.mid( start, token.length()-start-1 );
            continue;
        }
        else properties.append( { name, token } );
    }
    return properties;
}
