/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include "basedebugger.h"

class ScriptCpu;

class asDebugger : public BaseDebugger
{
    public:
        asDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~asDebugger();

        virtual bool upload() override;
        virtual int  compile( bool debug ) override;

    protected:
        //virtual bool postProcess() override;

    private:
        ScriptCpu* m_device;
};

#endif
