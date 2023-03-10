/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class BaseDebugger;
class LineNumberArea;
class Highlighter;
class OutPanelText;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

    public:
        CodeEditor( QWidget* parent, OutPanelText* outPane );
        ~CodeEditor();

        void setFile(const QString filePath);
        QString getFilePath() { return m_file ; }

        void setSyntaxFile( QString file );

        int debugLine() { return m_debugLine; }
        void setDebugLine( int line ) { m_debugLine = line; }

        void addBreakPoint( int line );
        void addError( int e )   { if( !m_errors.contains( e )   ) m_errors.append( e ); }
        void addWarning( int w ) { if( !m_warnings.contains( w ) ) m_warnings.append( w );}

        QList<int>* getBreakPoints() { return &m_brkPoints; }
        QList<int>* getErrors()   { return &m_errors; }
        QList<int>* getWarnings() { return &m_warnings; }

        void startDebug();
        void stopDebug();

        void lineNumberAreaPaintEvent( QPaintEvent* event );
        int  lineNumberAreaWidth();

        BaseDebugger* getCompiler() { return m_compiler; }
        void setCompiler( BaseDebugger* comp );

        QList<int> getFound();
        void setFound( QList<QTextEdit::ExtraSelection> sel );

        void showMsg( QString msg );

        void updateScreen();

 static void readSettings();
 static int  fontSize() { return m_fontSize; }
 static void setFontSize( int size );

 static int  tabSize() { return m_tabSize; }
 static void setTabSize( int size );

 static bool showSpaces() { return m_showSpaces; }
 static void setShowSpaces( bool on );

 static bool spaceTabs() { return m_spaceTabs; }
 static void setSpaceTabs( bool on );

        void toggleBreak() { m_brkAction = 3; }

    public slots:
        void slotAddBreak() { m_brkAction = 1; }
        void slotRemBreak() { m_brkAction = 2; }
        void slotClearBreak() { m_brkPoints.clear(); }
        bool compile( bool debug=false );

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
        QString changeCompilerFromCode();
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

 static bool m_showSpaces;
 static bool m_spaceTabs;

 static QStringList m_picInstr;
 static QStringList m_avrInstr;
 static QStringList m_i51Instr;

 static int m_fontSize;
 static int m_tabSize;

 static QString m_tab;

 static QFont m_font;

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
