/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include <QMenu>

#include "codeeditor.h"
#include "outpaneltext.h"

class QTabWidget;
class QToolBar;
class CodeEditor;
class EditorProp;
class FindReplaceDialog;

class EditorWidget : public QWidget
{
    Q_OBJECT

    public:
        EditorWidget( QWidget *parent );
        ~EditorWidget();

        bool close();

       CodeEditor* getCodeEditor();

    public slots:
       void loadFile( const QString &fileName );
       bool save();
       virtual void pause(){;}
       virtual void stop(){;}
       virtual void run(){;}

    protected slots:
        void confEditor();
        void confCompiler();
        void openRecentFile();
        void newFile();
        void open();
        bool saveAs();
        void closeTab(int);
        void documentWasModified();
        void tabChanged( int tab );
        void reload();

        void cut()   { getCodeEditor()->cut(); }
        void copy()  { getCodeEditor()->copy(); }
        void paste() { getCodeEditor()->paste(); }
        void undo()  { getCodeEditor()->undo(); }
        void redo()  { getCodeEditor()->redo(); }

        virtual void debug(){;}
        virtual void step(){;}
        virtual void stepOver(){;}
        void reset() { getCodeEditor()->reset(); }
        void compile() { getCodeEditor()->compile(); } /// m_compiler.compile( getCodeEditor()->getFilePath() );

        void upload() { getCodeEditor()->upload(); }
        void findReplaceDialog();

    protected:
        void dropEvent( QDropEvent* event );
        void dragEnterEvent( QDragEnterEvent* event);
        void keyPressEvent( QKeyEvent* event );

        void createWidgets();
        void createActions();
        void createToolBars();

        void readSettings();
        void writeSettings();

        void enableFileActs( bool enable );
        void enableDebugActs( bool enable );
        void setStepActs();

        enum { MaxRecentFiles = 10 };
        void updateRecentFileActions();

        bool maybeSave();
        bool saveFile( const QString &fileName );

        OutPanelText m_outPane;

        QString     m_lastDir;
        QStringList m_fileList;

        QTabWidget*  m_docWidget;

        FindReplaceDialog* findRepDiaWidget;
        EditorProp* m_editDialog;
        
        QMenu m_settingsMenu;
        QMenu m_fileMenu;

        QToolBar* m_editorToolBar;
        QToolBar* m_debuggerToolBar;

        QAction* confEditAct;
        QAction* confCompAct;
        QAction* recentFileActs[MaxRecentFiles];
        QAction* newAct;
        QAction* openAct;
        QAction* saveAct;
        QAction* saveAsAct;
        QAction* exitAct;
        QAction* aboutAct;
        QAction* aboutQtAct;
        QAction* undoAct;
        QAction* redoAct;

        QAction* cutAct;
        QAction* copyAct;
        QAction* pasteAct;
        
        QAction* debugAct;
        
        QAction* stepAct;
        QAction* stepOverAct;
        QAction* runAct;
        QAction* pauseAct;
        QAction* resetAct;
        QAction* stopAct;
        QAction* compileAct;
        QAction* loadAct;
        QAction* findQtAct;
};

#endif
