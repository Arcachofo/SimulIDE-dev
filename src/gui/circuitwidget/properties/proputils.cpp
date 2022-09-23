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
