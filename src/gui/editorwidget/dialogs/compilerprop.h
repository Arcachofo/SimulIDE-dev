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

        void on_driveCirc_toggled( bool drive );

    private:
        void updateDialog();

        CodeEditor* m_document;
        Compiler* m_compiler;

        bool m_blocked;
};


#endif
