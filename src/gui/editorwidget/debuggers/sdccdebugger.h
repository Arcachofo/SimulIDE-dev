/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SDCCDEBUGGER_H
#define SDCCDEBUGGER_H

#include "cdebugger.h"

class SdccDebugger : public cDebugger
{
    public:
        SdccDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~SdccDebugger();

        virtual int compile( bool debug ) override;

    protected:
        virtual bool postProcess() override;

        bool findCSEG();
};

#endif
