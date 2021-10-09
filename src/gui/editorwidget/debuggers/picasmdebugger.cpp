/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

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

