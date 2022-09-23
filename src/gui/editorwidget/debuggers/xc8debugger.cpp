/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "xc8debugger.h"

Xc8Debugger::Xc8Debugger( CodeEditor* parent, OutPanelText* outPane )
           : AvrGccDebugger( parent, outPane )
{
    m_addrBytes = 2;
}
Xc8Debugger::~Xc8Debugger(){}
