/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "compilerprop.h"
#include "codeeditor.h"
#include "basedebugger.h"
#include "editorwindow.h"
#include "mainwindow.h"

CompilerProp::CompilerProp( CodeEditor* parent )
            : QDialog( parent )
{
    setupUi(this);
    m_document = parent;

    setPathButton->setMaximumWidth( setPathButton->height() );
    setInclButton->setMaximumWidth( setInclButton->height() );

    m_blocked = true;
    compilerBox->insertItem( 0, "None" );
    compilerBox->insertItems( 1, EditorWindow::self()->compilers() );
    compilerBox->insertSeparator( compilerBox->count() );
    compilerBox->insertItems( compilerBox->count(), EditorWindow::self()->assemblers() );
    m_blocked = false;
    
    double scale = MainWindow::self()->fontScale();
    this->resize( 300*scale, 200*scale );
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

/*void CompilerProp::setBoard( int index )
{
    ardBoard->setCurrentIndex( index );
}*/

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
    QString board = ardBoard->itemText( index );
    bool showCustom = board == "Custom";
    customLabel->setVisible( showCustom  );
    customBoard->setVisible( showCustom  );
}

void CompilerProp::on_customBoard_textEdited( QString board )
{
    m_compiler->setProperty( "Custom_Board", board );
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
            bool showCustom = false;
            QString board = value.toString();
            if( board == "Custom" ) showCustom = true;
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
    }
    m_blocked = false;
}
