/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include"strprop.h"

QString StrBaseProp::getStr( QString str )
{
    return str.replace("\n","&#xa;").replace("\"","&#x22;")
            .replace("<","&#x3C").replace("=","&#x3D").replace(">","&#x3E");
}

QString StrBaseProp::setStr( QString str )
{
    return str.replace("&#xa;","\n").replace("&#x22;","\"")
            .replace("&#x3C","<").replace("&#x3D","=").replace("&#x3E",">");
}

double StrBaseProp::getValue()
{
    return getValStr().toDouble();
}

