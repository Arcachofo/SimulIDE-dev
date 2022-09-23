/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef APPIFACE_H
#define APPIFACE_H

#include <QObject>

class AppIface
{
    public:
        virtual ~AppIface() {}
        virtual void initialize() = 0;
        virtual void terminate() = 0;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(AppIface, "com.SimulIDE.Plugin.AppIface/1.0");
QT_END_NAMESPACE

#endif

