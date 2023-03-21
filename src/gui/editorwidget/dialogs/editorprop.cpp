/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "editorprop.h"
#include "codeeditor.h"
#include "basedebugger.h"
#include "mainwindow.h"

EditorProp::EditorProp( QWidget* parent )
          : QDialog( parent )
{
    setupUi(this);

    fontSize->setValue( CodeEditor::fontSize() );
    tabSize->setValue( CodeEditor::tabSize() );
    tabSpaces->setChecked( CodeEditor::spaceTabs() );
    showSpaces->setChecked( CodeEditor::showSpaces() );
    
    double scale = MainWindow::self()->fontScale();
    this->resize( 200*scale, 150*scale );
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

