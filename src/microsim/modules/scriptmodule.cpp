/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "scriptmodule.h"
#include "mcu.h"

ScriptModule::ScriptModule( eMcu* mcu, QString name )
            : ScriptBase( mcu->getId()+"-"+"ScriptModule" )
            , McuModule( mcu, name )
{

}
ScriptModule::~ScriptModule()
{
}
