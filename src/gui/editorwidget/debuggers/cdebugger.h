/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CDEBUGGER_H
#define CDEBUGGER_H

#include "basedebugger.h"

class cDebugger : public BaseDebugger
{
    public:
        cDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~cDebugger();

    protected:
        virtual void preProcess() override;
};

#endif
