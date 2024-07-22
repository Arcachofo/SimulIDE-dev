/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PROPUTILS_H
#define PROPUTILS_H

#include <QPointF>
#include <QStringRef>

QPointF getPointF( QString p );

QString getStrPointF( QPointF p );


//---------------------------------------------------

struct propStr_t{
    QStringRef name;
    QStringRef value;
};

QVector<propStr_t> parseXmlProps( QStringRef line );
QVector<propStr_t> parseProps( QStringRef line );
propStr_t parseProp( QStringRef token );

#endif
