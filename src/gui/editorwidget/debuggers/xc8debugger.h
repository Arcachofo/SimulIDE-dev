/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef XC8DEBUGGER_H
#define XC8DEBUGGER_H

#include "avrgccdebugger.h"

class Xc8Debugger : public AvrGccDebugger
{
    public:
        Xc8Debugger( CodeEditor* parent, OutPanelText* outPane );
        ~Xc8Debugger();
};

#endif
