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
    compilerBox->insertItems( 0, EditorWindow::self()->compilers() );
    m_blocked = false;
}

void CompilerProp::on_compilerBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;

    QString compName = compilerBox->itemText( index );
    BaseDebugger* deb = EditorWindow::self()->createDebugger( compName, m_document );
    setDebugger( deb );
    m_document->setDebugger( deb );
    if( compName != "None" )updateDialog();
}

void CompilerProp::on_setPathButton_clicked()
{
    m_debugger->getToolPath();
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
    m_debugger->setToolPath( path );
}

void CompilerProp::on_inclPath_editingFinished()
{
    QString path = inclPath->text();
    m_debugger->setIncludePath( path );
}

void CompilerProp::on_deviceText_editingFinished()
{
    QString dev = deviceText->text();
    m_debugger->setDevice( dev );
}

void CompilerProp::setDevice( QString dev )
{
    deviceText->setText( dev );
}

void CompilerProp::on_familyText_editingFinished()
{
    QString fam = familyText->text();
    m_debugger->setFamily( fam );
}

void CompilerProp::setFamily( QString fam )
{
    familyText->setText( fam );
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
    m_document->setDriveCirc( drive );
}

void CompilerProp::updateDialog()
{
    toolPathLabel->setVisible( true );
    toolPath->setVisible( true );
    setPathButton->setVisible( true );

    inclPathLabel->setVisible( true );
    inclPath->setVisible( true );
    setInclButton->setVisible( true );

    bool useFamily = m_debugger->useFamily();
    familyText->setVisible( useFamily );
    familyLabel->setVisible( useFamily );

    bool useDevice = m_debugger->useDevice();
    deviceText->setVisible( useDevice );
    deviceLabel->setVisible( useDevice );

    toolPath->setText( m_debugger->toolPath() );
    inclPath->setText( m_debugger->includePath() );
}

void CompilerProp::setDebugger( BaseDebugger* debugger )
{
    m_blocked = true;
    m_debugger = debugger;

    QString compiler = debugger->compName();
    compilerBox->setCurrentText( compiler );

    if( compiler != "None" )
    {
        updateDialog();

        driveCirc->setChecked( m_document->driveCirc() );

        QVariant value = debugger->property( "Board" );
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
            customBoard->setText( debugger->property( "Custom_Board" ).toString() );
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
