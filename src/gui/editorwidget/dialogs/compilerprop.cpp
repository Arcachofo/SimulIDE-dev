/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "compilerprop.h"
#include "propdialog.h"
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
    this->setMinimumWidth( 300*scale );
}

void CompilerProp::on_compilerBox_currentIndexChanged( int index )
{
    if( m_blocked ) return;

    QString compName = compilerBox->itemText( index );
    BaseDebugger* comp = EditorWindow::self()->createDebugger( compName, m_document );
    setCompiler( comp );
    m_document->setCompiler( comp );
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

    this->adjustSize();
}

void CompilerProp::setCompiler( Compiler* compiler )
{
    m_blocked = true;
    m_compiler = compiler;

    QString compName = compiler->compName();
    compilerBox->setCurrentText( compName );

    if( compName == "Arduino" ) inclPathLabel->setText( tr("Libraries Path") );
    else                        inclPathLabel->setText( tr("Include Path") );

    if( compName == "None" )
    {
        toolPathLabel->setVisible( false );
        toolPath->setVisible( false );
        setPathButton->setVisible( false );

        inclPathLabel->setVisible( false );
        inclPath->setVisible( false );
        setInclButton->setVisible( false );
    }else{
        PropDialog* pd = m_compiler->compilerProps();

        for( PropVal* propWidget : pd->propWidgets() )
            verticalLayout->addWidget( (QWidget*)propWidget );

        updateDialog();
    }
    m_blocked = false;
}
