/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>
#include <QMenu>

#include "compbase.h"
#include "codeeditor.h"
#include "outpaneltext.h"

class QTabWidget;
class QToolBar;
class CodeEditor;
class FindReplace;

class EditorWidget : public QWidget, public CompBase
{
    Q_OBJECT

    public:
        EditorWidget( QWidget* parent );
        ~EditorWidget();

        int  fontSize() { return m_fontSize; }
        void setFontSize( int size );

        int  tabSize() { return m_tabSize; }
        void setTabSize( int size );

        bool showSpaces() { return m_showSpaces; }
        void setShowSpaces( bool show );

        bool spaceTabs() { return m_spaceTabs; }
        void setSpaceTabs( bool on );

        bool close();

       CodeEditor* getCodeEditor();

       QStringList getFiles() { return m_fileList.keys(); }
       void restoreFile( QString filePath );

    public slots:
       void loadFile( QString filePath );
       bool save();
       virtual void pause(){;}
       virtual void stop(){;}
       virtual void run(){;}
       void findReplaceDialog();
       void reload();
       void updateDoc( int tab=0 );

    protected slots:
        void confEditor();
        void confFile();
        void confCompiler();
        void openRecentFile();
        void newFile();
        void open();
        bool saveAs();
        void closeTab(int);
        void documentWasModified();

        void cut()   { getCodeEditor()->cut(); }
        void copy()  { getCodeEditor()->copy(); }
        void paste() { getCodeEditor()->paste(); }
        void undo()  { getCodeEditor()->undo(); }
        void redo()  { getCodeEditor()->redo(); }

        virtual void debug(){;}
        virtual void step(){;}
        virtual void stepOver(){;}
        virtual void reset() {;}
        void compile() { getCodeEditor()->compile(); } /// m_compiler.compile( getCodeEditor()->getFilePath() );

        virtual bool upload() {return false;}

    protected:
        void docShowSpaces( CodeEditor* ce );

        QList<CodeEditor*> getCodeEditors();

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
        void setStepActs( bool s );

        enum { MaxRecentFiles = 10 };
        void addRecentFile( QString filePath );
        void updateRecentFileActions();

        bool maybeSave();
        bool saveFile( QString fileName );

        void addDocument( QString file, bool main );

        QFont m_font;
        int m_fontSize;
        int m_tabSize;

        QString m_tab;

        bool m_showSpaces;
        bool m_spaceTabs;

        OutPanelText m_outPane;

        QString     m_lastDir;
        QHash<QString, QWidget*> m_fileList;

        QTabWidget*  m_docWidget;
        FindReplace* m_findRepDialog;

        QMenu m_settingsMenu;
        QMenu m_fileMenu;

        QToolBar* m_editorToolBar;
        QToolBar* m_findToolBar;
        QToolBar* m_compileToolBar;
        QToolBar* m_debuggerToolBar;

        QAction* confEditAct;
        QAction* confFileAct;
        QAction* confCompAct;

        QAction* recentFileActs[MaxRecentFiles];
        QAction* newAct;
        QAction* openAct;
        QAction* saveAct;
        QAction* saveAsAct;
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
