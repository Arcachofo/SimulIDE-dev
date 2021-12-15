/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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
