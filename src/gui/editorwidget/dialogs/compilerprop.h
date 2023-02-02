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
        void setFamily( QString fam );
        void setDevice( QString dev );
        //void setBoard( int index );

    public slots:
        void on_compilerBox_currentIndexChanged( int index );

        void on_setPathButton_clicked();
        void on_setInclButton_clicked();

        void on_toolPath_editingFinished();
        void on_inclPath_editingFinished();

        void on_familyText_editingFinished();
        void on_deviceText_editingFinished();

        void on_ardBoard_currentIndexChanged( int index );
        void on_customBoard_textEdited( QString board );

    private:
        void updateDialog();

        CodeEditor* m_document;
        Compiler* m_compiler;

        bool m_blocked;
};


#endif
