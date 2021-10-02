/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */
 
/***************************************************************************
 *   Modified 2012 by santiago González                                    *
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

#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

#include "findreplace_global.h"

namespace Ui {
    class FindReplaceDIALOG;
}

class QTextEdit;
class QSettings;

class FindReplaceDialog : public QDialog
{
    Q_OBJECT
    public:
        FindReplaceDialog(QWidget* parent=0);
        virtual ~FindReplaceDialog();

        void setTextEdit(CodeEditor *textEdit);

        virtual void writeSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");
        virtual void readSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");

        void setTextToFind( QString text);

    public slots:
        void findNext();
        void findPrev();

    protected:
        void changeEvent (QEvent* e );

        Ui::FindReplaceDIALOG* ui;
};

#endif // FINDREPLACEDIALOG_H
