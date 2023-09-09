/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

#include "compbase.h"

class BaseDebugger;
class LineNumberArea;
class Highlighter;
class OutPanelText;

class CodeEditor : public QPlainTextEdit, public CompBase
{
    Q_OBJECT

    public:
        CodeEditor( QWidget* parent, OutPanelText* outPane );
        ~CodeEditor();

        void setFile( QString filePath );
        QString getFile() { return m_file ; }

        QString compName();
        void setCompName( QString n );

        bool saveAtClose() { return m_saveAtClose; }
        void setSaveAtClose( bool s ) { m_saveAtClose = s; }

        bool loadCompiler() { return m_loadCompiler; }
        void setLoadCompiler( bool l ) { m_loadCompiler = l; }

        bool loadBreakp() { return m_loadBreakp; }
        void setLoadBreakp( bool l ) { m_loadBreakp = l; }

        bool openFiles() { return m_openFiles; }
        void setOpenFiles( bool o ) { m_openFiles = o; }

        QString circuit();
        void setCircuit( QString c );

        QString breakpoints();
        void setBreakpoints( QString bp );

        QString fileList();
        void setFileList( QString fl );

        void setSyntaxFile( QString file );

        void loadConfig();
        void saveConfig();

        void closing() { if( m_saveAtClose ) saveConfig(); }

        int debugLine() { return m_debugLine; }
        void setDebugLine( int line ) { m_debugLine = line; }

        void addBreakPoint( int line );
        void addError( int e )   { if( !m_errors.contains( e )   ) m_errors.append( e ); }
        void addWarning( int w ) { if( !m_warnings.contains( w ) ) m_warnings.append( w );}

        QList<int>* getBreakPoints() { return &m_brkPoints; }
        QList<int>* getErrors()      { return &m_errors; }
        QList<int>* getWarnings()    { return &m_warnings; }

        void startDebug();
        void stopDebug();

        void lineNumberAreaPaintEvent( QPaintEvent* event );
        int  lineNumberAreaWidth();

        BaseDebugger* getCompiler() { return m_compiler; }
        //void setCompiler( BaseDebugger* comp );

        void fileProps();

        QList<int> getFound();
        void setFound( QList<QTextEdit::ExtraSelection> sel );

        void showMsg( QString msg );

        void updateScreen();

        void toggleBreak() { m_brkAction = 3; }

        bool compile( bool debug=false );

    public slots:
        void slotAddBreak() { m_brkAction = 1; }
        void slotRemBreak() { m_brkAction = 2; }
        void slotClearBreak() { m_brkPoints.clear(); }

    private slots:
        void updateLineNumberAreaWidth(int) { setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 ); }
        void updateLineNumberArea( const QRect &, int );
        void highlightCurrentLine();
        void deleteSelected();

    protected:
        void resizeEvent( QResizeEvent* event );
        void keyPressEvent( QKeyEvent* event );
        void contextMenuEvent( QContextMenuEvent* event );

    private:
        int  getSyntaxCoincidences();
        void remBreakPoint( int line );

        void indentSelection( bool unIndent );

        BaseDebugger* m_compiler;
        OutPanelText* m_outPane;

        LineNumberArea* m_lNumArea;
        Highlighter*    m_hlighter;

        QList<QTextEdit::ExtraSelection> m_found;

        QString m_file;
        QString m_help;

        QList<int> m_brkPoints;
        QList<int> m_errors;
        QList<int> m_warnings;

        int m_brkAction;    // 0 = no action, 1 = add brkpoint, 2 = rem brkpoint
        int m_debugLine;
        int m_numLines;

        bool m_saveAtClose;
        bool m_openFiles;
        bool m_openCircuit;
        bool m_loadCompiler;
        bool m_loadBreakp;

        QString m_tab;
        QFont m_font;

 static QStringList m_picInstr;
 static QStringList m_avrInstr;
 static QStringList m_i51Instr;

 static QList<CodeEditor*> m_documents;
};


// ********************* CLASS LineNumberArea **********************************

class LineNumberArea : public QWidget
{
    Q_OBJECT
    
    public:
        LineNumberArea( CodeEditor* editor );
        ~LineNumberArea();

        QSize sizeHint() const { return QSize( m_codeEditor->lineNumberAreaWidth(), 0 ); }

        int lastPos;

    protected:
        void contextMenuEvent( QContextMenuEvent* event);
        void mousePressEvent( QMouseEvent* event );
        void paintEvent( QPaintEvent* event ) { m_codeEditor->lineNumberAreaPaintEvent( event ); }

    private:
        CodeEditor* m_codeEditor;
};

/*#include <QTextBlockUserData>
class UserData : public QTextBlockUserData
{
    public:
        UserData()
        {
            breakp = false;
            error   = false;
            warning = false;
        }
        ~UserData(){;}

        bool breakp;
        bool error;
        bool warning;

};*/
#endif
