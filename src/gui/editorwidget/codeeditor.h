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

        int debugLine() { return m_debugLine; }
        void setDebugLine( int line ) { m_debugLine = line; }

        bool hasBreakPoint( int line ) { return m_brkPoints.contains( line ); }
        void startDebug();

        void lineNumberAreaPaintEvent( QPaintEvent* event );
        int  lineNumberAreaWidth();

        BaseDebugger* getCompiler() { return m_compiler; }
        void setCompiler( BaseDebugger* comp );

        void updateScreen();

 static void readSettings();
 static int fontSize() { return m_fontSize; }
 static void setFontSize( int size );

 static int tabSize() { return m_tabSize; }
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

    protected:
        void resizeEvent( QResizeEvent* event );
        void keyPressEvent( QKeyEvent* event );
        void contextMenuEvent(QContextMenuEvent* event);

    private:
        int  getSintaxCoincidences();
        void addBreakPoint( QTextBlock* block );
        void remBreakPoint( QTextBlock* block );

        void indentSelection( bool unIndent );

        BaseDebugger* m_compiler;
        OutPanelText* m_outPane;

        LineNumberArea* m_lNumArea;
        Highlighter*    m_hlighter;

        QString m_file;
        QString m_help;

        QList<int> m_brkPoints;

        int m_brkAction;    // 0 = no action, 1 = add brkpoint, 2 = rem brkpoint
        int m_debugLine;
        int m_errorLine;

 static bool m_showSpaces;
 static bool m_spaceTabs;

 static QStringList m_picInstr;
 static QStringList m_avrInstr;
 static QStringList m_i51Instr;

 static int   m_fontSize;
 static int   m_tabSize;

 static QString m_sintaxPath;
 static QString m_compilsPath;
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

#endif
