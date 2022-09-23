/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICASMDEBUGGER_H
#define PICASMDEBUGGER_H

#include "basedebugger.h"

class PicAsmDebugger : public BaseDebugger
{
    public:
        PicAsmDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~PicAsmDebugger();

    protected:
        //virtual int compile( bool ) override;
        virtual bool postProcess() override;
};
#endif
