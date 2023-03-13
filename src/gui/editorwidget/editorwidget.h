/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
class FindReplace;

class EditorWidget : public QWidget
{
    Q_OBJECT

    public:
        EditorWidget( QWidget* parent );
        ~EditorWidget();

        bool close();

       CodeEditor* getCodeEditor();


    public slots:
       void loadFile( const QString &fileName );
       bool save();
       virtual void pause(){;}
       virtual void stop(){;}
       virtual void run(){;}
       void findReplaceDialog();
       void reload();

    protected slots:
        void confEditor();
        void confCompiler();
        void openRecentFile();
        void newFile();
        void open();
        bool saveAs();
        void closeTab(int);
        void documentWasModified();
        //void tabChanged( int tab );


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
        void updateRecentFileActions();

        bool maybeSave();
        bool saveFile( const QString &fileName );
        void saveBreakpoints( QString fileName );
        void loadBreakpoints( const QString &fileName );

        OutPanelText m_outPane;

        QString     m_lastDir;
        QHash<QString, QWidget*> m_fileList;

        QTabWidget*  m_docWidget;

        FindReplace* m_findRepDialog;
        EditorProp* m_editDialog;

        QMenu m_settingsMenu;
        QMenu m_fileMenu;

        QToolBar* m_editorToolBar;
        QToolBar* m_findToolBar;
        QToolBar* m_compileToolBar;
        QToolBar* m_debuggerToolBar;

        QAction* confEditAct;
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
