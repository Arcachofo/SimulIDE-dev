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

#include "compilerprop.h"
#include "codeeditor.h"
#include "basedebugger.h"
#include "editorwindow.h"

CompilerProp::CompilerProp( CodeEditor* parent, BaseDebugger* debugger )
            : QDialog( parent )
{
    setupUi(this);
    m_blocked = true;

    m_editor   = parent;
    m_debugger = debugger;

    QString compiler = debugger->compName();

    if( !compiler.isEmpty() )
    {
        compilerBox->insertItem( 0, compiler );

        updateDialog();

        driveCirc->setChecked( m_editor->driveCirc() );

        QVariant value = debugger->property( "Board" );
        if( !value.isValid() )
        {
            boardLabel->setVisible( false );
            ardBoard->setVisible( false );
            customLabel->setVisible( false );
            customBoard->setVisible( false );
        }else{
            ardBoard->setCurrentIndex( value.toInt() );
            customBoard->setText( debugger->property( "Custom_Board" ).toString() );
    }   }
    else{
        compilerBox->insertItem( 0, "None");
        compilerBox->insertItems( 1, EditorWindow::self()->compilers() );

        toolPathLabel->setVisible( false );
        toolPath->setVisible( false );
        setPathButton->setVisible( false );

        inclPathLabel->setVisible( false );
        inclPath->setVisible( false );
        setInclButton->setVisible( false );

        device->setVisible( false );
        deviceLabel->setVisible( false );

        boardLabel->setVisible( false );
        ardBoard->setVisible( false );
        customLabel->setVisible( false );
        customBoard->setVisible( false );
        driveCirc->setVisible( false );
    }
    m_blocked = false;
}

void CompilerProp::on_compilerBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;
    EditorWindow::self()->loadCompiler( compilerBox->itemText( index), m_debugger );
    updateDialog();
}

void CompilerProp::on_setPathButton_clicked()
{
    m_debugger->getCompilerPath();
    updateDialog();
}

void CompilerProp::on_setInclButton_clicked()
{
    m_debugger->getIncludePath();
    updateDialog();
}

void CompilerProp::on_toolPath_editingFinished()
{
    QString path = toolPath->text();
    m_debugger->setCompilerPath( path );
}

void CompilerProp::on_inclPath_editingFinished()
{
    QString path = inclPath->text();
    m_debugger->setIncludePath( path );
}

void CompilerProp::on_device_editingFinished()
{
    QString dev = device->text();
    m_debugger->setDevice( dev );
}

void CompilerProp::on_ardBoard_currentIndexChanged( int index )
{
    m_debugger->setProperty( "Board", index );
}

void CompilerProp::on_customBoard_textEdited( QString board )
{
    m_debugger->setProperty( "Custom_Board", board );
}

void CompilerProp::on_driveCirc_toggled( bool drive )
{
    m_editor->setDriveCirc( drive );
}

void CompilerProp::updateDialog()
{
    toolPathLabel->setVisible( true );
    toolPath->setVisible( true );
    setPathButton->setVisible( true );

    inclPathLabel->setVisible( true );
    inclPath->setVisible( true );
    setInclButton->setVisible( true );

    bool useDevice = m_debugger->useDevice();
    device->setVisible( useDevice );
    deviceLabel->setVisible( useDevice );

    toolPath->setText( m_debugger->compilerPath() );
    inclPath->setText( m_debugger->includePath() );
}
