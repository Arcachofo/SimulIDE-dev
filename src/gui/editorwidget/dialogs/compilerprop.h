/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         */

#include <QDialog>

#include "ui_compilerprop.h"

#ifndef COMPILERDIALOG_H
#define COMPILERDIALOG_H

class CodeEditor;
class Compiler;

class CompilerProp : public QDialog, private Ui::CompilerProp
{
    Q_OBJECT
    
    public:
        CompilerProp( CodeEditor* parent );

        void setCompiler( Compiler* compiler );

    public slots:
        void on_compilerBox_currentIndexChanged( int index );

        void on_setPathButton_clicked();
        void on_setInclButton_clicked();

        void on_toolPath_editingFinished();
        void on_inclPath_editingFinished();

    private:
        CodeEditor* m_document;
        Compiler* m_compiler;

        bool m_blocked;
};


#endif
