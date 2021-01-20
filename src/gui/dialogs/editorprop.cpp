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

EditorProp::EditorProp( CodeEditor* parent, BaseDebugger* debugger )
          : QDialog( parent )
{
    setupUi(this);

    m_editor   = parent;
    m_debugger = debugger;

    fontSize->setValue( m_editor->fontSize() );
    tabSize->setValue( m_editor->tabSize() );
    tabSpaces->setChecked( m_editor->spaceTabs() );
    showSpaces->setChecked( m_editor->showSpaces() );

    if( m_debugger )
    {
        compPath->setText( debugger->compilerPath() );

        QVariant value = debugger->property( "Avra_Inc_Path" );
        if( !value.isValid() )
        {
            avraIncLabel->setVisible( false );
            avraInc->setVisible( false );
        }

        value = debugger->property( "Board" );
        if( !value.isValid() )
        {
            boardLabel->setVisible( false );
            ardBoard->setVisible( false );
            customLabel->setVisible( false );
            customBoard->setVisible( false );
            driveCirc->setVisible( false );
        }
    }else{
        compPathLabel->setVisible( false );
        compPath->setVisible( false );
    }
}

void EditorProp::on_fontSize_valueChanged( int size )
{
    m_editor->setFontSize( size );
}

void EditorProp::on_tabSize_valueChanged( int size )
{
    m_editor->setTabSize( size );
}

void EditorProp::on_tabSpaces_toggled( bool spaces )
{
    m_editor->setSpaceTabs( spaces );
}

void EditorProp::on_showSpaces_toggled( bool show )
{
    m_editor->setShowSpaces( show );
}

void EditorProp::on_driveCirc_toggled( bool drive )
{
    m_editor->setDriveCirc( drive );
}

void EditorProp::on_compPath_textEdited( QString path )
{
    m_debugger->setCompilerPath( path );
}

void EditorProp::on_avraInc_textEdited( QString path )
{
    m_debugger->setProperty( "Avra_Inc_Path", path );
}

void EditorProp::on_ardBoard_currentIndexChanged( int index )
{
    m_debugger->setProperty( "Board", index );
}

void EditorProp::on_customBoard_textEdited( QString board )
{
    m_debugger->setProperty( "Custom_Board", board );
}
