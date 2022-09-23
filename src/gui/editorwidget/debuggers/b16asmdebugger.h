/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef B16ASMDEBUGGER_H
#define B16ASMDEBUGGER_H

#include "basedebugger.h"

class B16AsmDebugger : public BaseDebugger
{
    Q_OBJECT

    public:
        B16AsmDebugger( CodeEditor* parent, OutPanelText* outPane) ;
        ~B16AsmDebugger();
        
        virtual int compile( bool ) override;

    private:
        virtual bool postProcess() override;
        void loadCodes();

        int m_codeBits;

        QHash<QString, int> m_opcodes;
};

#endif
