/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "picasmdebugger.h"
#include "gputilsdebug.h"

PicAsmDebugger::PicAsmDebugger( CodeEditor* parent, OutPanelText* outPane )
              : BaseDebugger( parent, outPane )
{
    m_typesList["byte"] = "uint8";
}
PicAsmDebugger::~PicAsmDebugger() {}

bool PicAsmDebugger::postProcess()
{
    return BaseDebugger::postProcess();
    //m_flashToSource.clear();
    //m_sourceToFlash.clear();

    //return GputilsDebug::mapFlashToAsm( this );
}

