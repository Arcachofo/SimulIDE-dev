/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "editorprop.h"
#include "codeeditor.h"
#include "basedebugger.h"

EditorProp::EditorProp( QWidget* parent )
          : QDialog( parent )
{
    setupUi(this);

    fontSize->setValue( CodeEditor::fontSize() );
    tabSize->setValue( CodeEditor::tabSize() );
    tabSpaces->setChecked( CodeEditor::spaceTabs() );
    showSpaces->setChecked( CodeEditor::showSpaces() );
}

void EditorProp::on_fontSize_valueChanged( int size )
{
    CodeEditor::setFontSize( size );
}

void EditorProp::on_tabSize_valueChanged( int size )
{
    CodeEditor::setTabSize( size );
}

void EditorProp::on_tabSpaces_toggled( bool spaces )
{
    CodeEditor::setSpaceTabs( spaces );
}

void EditorProp::on_showSpaces_toggled( bool show )
{
    CodeEditor::setShowSpaces( show );
}

