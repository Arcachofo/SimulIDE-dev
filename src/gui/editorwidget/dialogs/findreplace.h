/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDialog>

#include "ui_findreplace.h"

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

class CodeEditor;

class FindReplace : public QDialog, private Ui::FindReplace
{
    Q_OBJECT

    public:
        FindReplace( QWidget* parent );

        void setEditor( CodeEditor* ce ) { m_editor = ce; }
        void setTextToFind( QString text ) { findEdit->setText( text ); }

    public slots:

        //void on_findEdit_textEdited( QString text );
        //void on_replaceEdit_textEdited( QString text );

        void on_prevButton_clicked();
        void on_nextButton_clicked();
        void on_allButton_clicked();
        void on_replaceButton_clicked();
        void on_replFindButton_clicked();
        void on_replAllButton_clicked();
        void on_closeButton_clicked();


        void on_caseS_toggled( bool c );
        void on_whole_toggled( bool w );
        void on_regexp_toggled( bool w );

    private:
        bool find( bool next );
        void showMsg( QString msg );

        QTextCursor m_textCursor;
        CodeEditor* m_editor;
};

#endif
