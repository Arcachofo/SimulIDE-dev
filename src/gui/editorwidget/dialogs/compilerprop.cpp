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

CompilerProp::CompilerProp( CodeEditor* parent )
            : QDialog( parent )
{
    setupUi(this);
    m_document = parent;
    m_blocked = true;
    compilerBox->insertItem( 0, "None" );
    compilerBox->insertItems( 1, EditorWindow::self()->compilers() );
    compilerBox->insertSeparator( compilerBox->count() );
    compilerBox->insertItems( compilerBox->count(), EditorWindow::self()->assemblers() );
    m_blocked = false;
}

void CompilerProp::on_compilerBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;

    QString compName = compilerBox->itemText( index );
    BaseDebugger* comp = EditorWindow::self()->createDebugger( compName, m_document );
    setCompiler( comp );
    m_document->setCompiler( comp );
    if( compName != "None" ) updateDialog();
}

void CompilerProp::on_setPathButton_clicked()
{
    m_compiler->getToolPath();
    updateDialog();
}

void CompilerProp::on_setInclButton_clicked()
{
    m_compiler->getIncludePath();
    updateDialog();
}

void CompilerProp::on_toolPath_editingFinished()
{
    QString path = toolPath->text();
    m_compiler->setToolPath( path );
}

void CompilerProp::on_inclPath_editingFinished()
{
    QString path = inclPath->text();
    m_compiler->setIncludePath( path );
}

void CompilerProp::on_deviceText_editingFinished()
{
    QString dev = deviceText->text();
    m_compiler->setDevice( dev );
}

void CompilerProp::setDevice( QString dev )
{
    deviceText->setText( dev );
}

void CompilerProp::on_familyText_editingFinished()
{
    QString fam = familyText->text();
    m_compiler->setFamily( fam );
}

void CompilerProp::setFamily( QString fam )
{
    familyText->setText( fam );
}

void CompilerProp::on_ardBoard_currentIndexChanged( int index )
{
    m_compiler->setProperty( "Board", index );
}

void CompilerProp::on_customBoard_textEdited( QString board )
{
    m_compiler->setProperty( "Custom_Board", board );
}

void CompilerProp::on_driveCirc_toggled( bool drive )
{
    EditorWindow::self()->setDriveCirc( drive );
}

void CompilerProp::updateDialog()
{
    toolPathLabel->setVisible( true );
    toolPath->setVisible( true );
    setPathButton->setVisible( true );
    toolPath->setText( m_compiler->toolPath() );

    inclPathLabel->setVisible( true );
    inclPath->setVisible( true );
    setInclButton->setVisible( true );
    inclPath->setText( m_compiler->includePath() );

    bool useFamily = m_compiler->useFamily();
    familyText->setVisible( useFamily );
    familyLabel->setVisible( useFamily );
    familyText->setText( m_compiler->family() );

    bool useDevice = m_compiler->useDevice();
    deviceText->setVisible( useDevice );
    deviceLabel->setVisible( useDevice );
    deviceText->setText( m_compiler->device() );

    /// driveCirc->setVisible( true );
    driveCirc->setChecked( EditorWindow::self()->driveCirc() );
}

void CompilerProp::setCompiler( Compiler* compiler )
{
    m_blocked = true;
    m_compiler = compiler;

    QString compName = compiler->compName();
    compilerBox->setCurrentText( compName );

    if( compName != "None" )
    {
        updateDialog();

        QVariant value = compiler->property( "Board" );
        if( !value.isValid() )
        {
            boardLabel->setVisible( false );
            ardBoard->setVisible( false );
            customLabel->setVisible( false );
            customBoard->setVisible( false );
        }else{
            boardLabel->setVisible( true );
            ardBoard->setVisible( true );
            ardBoard->setCurrentIndex( value.toInt() );
            customLabel->setVisible( true );
            customBoard->setVisible( true );
            customBoard->setText( compiler->property( "Custom_Board" ).toString() );
    }   }
    else{
        toolPathLabel->setVisible( false );
        toolPath->setVisible( false );
        setPathButton->setVisible( false );

        inclPathLabel->setVisible( false );
        inclPath->setVisible( false );
        setInclButton->setVisible( false );

        familyText->setVisible( false );
        familyLabel->setVisible( false );

        deviceText->setVisible( false );
        deviceLabel->setVisible( false );

        boardLabel->setVisible( false );
        ardBoard->setVisible( false );
        customLabel->setVisible( false );
        customBoard->setVisible( false );
        driveCirc->setVisible( false );
    }
    m_blocked = false;
}
