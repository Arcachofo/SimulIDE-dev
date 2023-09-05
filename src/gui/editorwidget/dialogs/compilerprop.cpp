/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "compilerprop.h"
#include "propdialog.h"
#include "propval.h"
#include "codeeditor.h"
#include "basedebugger.h"
#include "editorwindow.h"
#include "mainwindow.h"

CompilerProp::CompilerProp( CodeEditor* parent )
            : QDialog( parent )
{
    setupUi(this);
    m_compiler = nullptr;
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
    this->setMinimumWidth( 250*scale );
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
}

void CompilerProp::on_setInclButton_clicked()
{
    m_compiler->getIncludePath();
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

void CompilerProp::setCompiler( Compiler* compiler )
{
    m_blocked = true;

    if( m_compiler ) {
        PropDialog* pd = m_compiler->compilerProps();
        for( QWidget* propWidget : pd->propWidgets() )
        {
            verticalLayout->removeWidget( propWidget );
            propWidget->setVisible(false); // hacking to layout
        }
    }
    m_compiler = compiler;

    QString compName = compiler->compName();
    compilerBox->setCurrentText( compName );

    toolPath->setText( m_compiler->toolPath() );
    inclPath->setText( m_compiler->includePath() );

    if( compName == "Arduino" ) inclPathLabel->setText( tr("Libraries Path") );
    else                        inclPathLabel->setText( tr("Include Path") );

    bool paths = compName != "None";

    toolPathLabel->setVisible( paths );
    toolPath->setVisible( paths );
    setPathButton->setVisible( paths );

    inclPathLabel->setVisible( paths );
    inclPath->setVisible( paths );
    setInclButton->setVisible( paths );

    if( paths )
    {
        PropDialog* pd = m_compiler->compilerProps();
        for( QWidget* propWidget : pd->propWidgets() )
            verticalLayout->addWidget( propWidget );
    }
    this->adjustSize();
    m_blocked = false;
}
