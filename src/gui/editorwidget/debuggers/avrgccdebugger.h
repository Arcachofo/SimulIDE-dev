/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRGCCDEBUGGER_H
#define AVRGCCDEBUGGER_H

#include "cdebugger.h"

class AvrGccDebugger : public cDebugger
{
    public:
        AvrGccDebugger( CodeEditor* parent, OutPanelText* outPane );
        ~AvrGccDebugger();

    protected:
        virtual bool postProcess() override;

        bool getVariables();
        bool getFunctions();
        bool mapFlashToSource();

        int m_addrBytes;

        QString m_elfPath;
};

#endif
