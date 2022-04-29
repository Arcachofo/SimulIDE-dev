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

#include <QPainter>
#include <QTextDocumentFragment>
#include <QSettings>

#include "codeeditor.h"
#include "outpaneltext.h"
#include "compilerprop.h"
#include "highlighter.h"
#include "basedebugger.h"
#include "mainwindow.h"
#include "editorwindow.h"
#include "utils.h"

QStringList CodeEditor::m_picInstr = QString("addlw addwf andlw andwf banksel bcf bov bsf btfsc btg btfss clrf clrw clrwdt comf decf decfsz goto incf incfsz iorlw iorwf movf movlw movwf reset retfie retlw return rlf rrf sublw subwf swapf xorlw xorwf").split(" ");
QStringList CodeEditor::m_avrInstr = QString("nop add adc adiw call inc sleep sub subi sbc sbci sbiw and andi or ori eor elpm fmul fmuls fmulsu mul muls smp com neg sbr cbr dec tst clr ser mul rjmp ijmp jmp rcall icall ret reti cpse cp cpc cpi sbrc sbrs sbic sbis brbs brbc breq brne brcs break brcc brsh brlo brmi brpl brge brlt brhs brhc brts brtc brvs brvc brie brid mov movw ldi lds ld ldd sts st std lpm in out push pop lsl lsr rol ror asr swap bset bclr sbi cbi bst bld sec clc sen cln sez clz sei cli ses cls sev clv set clt seh clh wdr des eicall eijmp lac las lat spm xch").split(" ");
QStringList CodeEditor::m_i51Instr = QString("nop ajmp ljmp rr inc jbc acall lcall rrc dec jb ajmp ret rl add jnb reti rlc addc jc jnz orl jmp jnc anl jz xrl mov sjmp ajmp movc subb mul cpl cjne push clr swap xch pop setb da djnz xchd movx").split(" ");

bool    CodeEditor::m_showSpaces = false;
bool    CodeEditor::m_spaceTabs  = false;
int     CodeEditor::m_fontSize = 13;
int     CodeEditor::m_tabSize = 4;
QString CodeEditor::m_sintaxPath;
QString CodeEditor::m_compilsPath;
QString CodeEditor::m_tab;
QFont   CodeEditor::m_font = QFont();

QList<CodeEditor*> CodeEditor::m_documents;

CodeEditor::CodeEditor( QWidget* parent, OutPanelText* outPane )
          : QPlainTextEdit( parent )
{
    m_documents.append( this );

    m_outPane   = outPane;
    m_lNumArea  = new LineNumberArea( this );
    m_hlighter  = new Highlighter( document() );
    
    m_compiler = NULL;
    m_debugLine = 0;
    m_brkAction = 0;
    m_help = "";

    setFont( m_font );
    setFontSize( m_fontSize );
    setTabSize( m_tabSize );
    setShowSpaces( m_showSpaces );
    setSpaceTabs( m_spaceTabs );

    QPalette p = palette();
    p.setColor( QPalette::Base, QColor( 255, 255, 249) );
    p.setColor( QPalette::Text, QColor( 0, 0, 0) );
    setPalette( p );

    connect( this, SIGNAL( blockCountChanged( int )),
             this, SLOT( updateLineNumberAreaWidth( int )), Qt::UniqueConnection );

    connect( this, SIGNAL( updateRequest( QRect,int )),
             this, SLOT( updateLineNumberArea( QRect,int )), Qt::UniqueConnection);

    connect( this, SIGNAL( cursorPositionChanged() ),
             this, SLOT( highlightCurrentLine() ), Qt::UniqueConnection);
    
    setLineWrapMode( QPlainTextEdit::NoWrap );
    updateLineNumberAreaWidth( 0 );
    highlightCurrentLine();
}
CodeEditor::~CodeEditor()
{
    m_documents.removeAll( this );
}

void CodeEditor::setCompiler( BaseDebugger* compiler )
{
    if( m_compiler ) delete m_compiler;
    m_compiler = compiler;
}

void CodeEditor::setFile( const QString filePath )
{
    if( m_file == filePath ) return;

    if( m_compiler ) delete m_compiler;
    m_compiler = NULL;

    m_outPane->appendLine( "-------------------------------------------------------" );
    m_outPane->appendLine( tr(" File: ")+filePath+"\n" );

    m_file = filePath;
    QDir::setCurrent( m_file );

    QString extension = getFileExt( filePath );

    QString code = "00";
    if( extension == ".gcb" )
    {
        m_hlighter->readSintaxFile( m_sintaxPath + "gcbasic.sintax" );
        m_compiler = EditorWindow::self()->createDebugger( "GcBasic", this );
    }
    else if( extension == ".cpp"
          || extension == ".c"
          || extension == ".ino"
          || extension == ".h" )
    {
        m_hlighter->readSintaxFile( m_sintaxPath + "cpp.sintax" );
        if( extension == ".ino" )
            m_compiler = EditorWindow::self()->createDebugger( "Arduino", this );
        else if( extension == ".cpp" || extension == ".c")
            code = "10";
    }
    else if( extension == ".s" )
    {
        m_hlighter->readSintaxFile( m_sintaxPath + "avrasm.sintax" );
        m_compiler = EditorWindow::self()->createDebugger( "Avrgcc-asm", this );
    }
    else if( extension == ".asm"  // We should identify if pic, avr or i51 asm
          || extension == ".a51" )
    {
        m_outPane->appendText( tr("File recognized as: ") );

        int type = getSintaxCoincidences();
        if( type == 1 )   // Is Pic
        {
            m_outPane->appendLine( "Pic asm\n" );
            m_hlighter->readSintaxFile( m_sintaxPath + "pic14asm.sintax" );
            m_compiler = EditorWindow::self()->createDebugger( "GpAsm", this );
        }
        else if( type == 2 )  // Is Avr
        {
                m_outPane->appendLine( "Avr asm\n" );
                m_hlighter->readSintaxFile( m_sintaxPath + "avrasm.sintax" );
                m_compiler = EditorWindow::self()->createDebugger( "Avra", this );
        }
        else if( type == 3 )  // Is 8051
        {
                m_outPane->appendLine( "I51 asm\n" );
                m_hlighter->readSintaxFile( m_sintaxPath + "i51asm.sintax" );
        }
        else m_outPane->appendLine( "Unknown asm\n" );
    }
    else if( extension == ".xml"
         ||  extension == ".html"
         ||  extension == ".package"
         ||  extension == ".mcu"
         ||  extension == ".sim1"
         ||  extension == ".simu" )
    {
        m_hlighter->readSintaxFile( m_sintaxPath + "xml.sintax" );
    }
    else if( getFileName( m_file ).toLower() == "makefile"  )
    {
        m_hlighter->readSintaxFile( m_sintaxPath + "makef.sintax" );
    }
    else if( extension == ".hex"
         ||  extension == ".ihx" )
    {
        m_font.setLetterSpacing( QFont::PercentageSpacing, 110 );
        setFont( m_font );
        m_hlighter->readSintaxFile( m_sintaxPath + "hex.sintax" );
    }
    else if( extension == ".sac" )
    {
        //m_compiler = new B16AsmDebugger( this, m_outPane );
    }
    QStringList lines = fileToStringList( filePath, "CodeEditor::setFile" );
    QString line = lines.first();
    if( !m_compiler )
    {
        QString compiler = BaseDebugger::getValue( line, "compiler" );
        if( compiler.isEmpty() ) compiler = "None";
        else m_outPane->appendLine( tr("Found Compiler definition in file: ") + compiler );

        m_compiler = EditorWindow::self()->createDebugger( compiler, this, code );
    }
    m_compiler->getInfoInFile( line );
    m_outPane->appendLine( "-------------------------------------------------------" );
}

int CodeEditor::getSintaxCoincidences()
{
    QStringList lines = fileToStringList( m_file, "CodeEditor::getSintaxCoincidences" );

    double avr=1, pic=1, i51=1; // Avoid divide by 0
    int nlines = 0;

    for( QString line : lines )
    {
        QStringList words = line.toLower().replace("\t", " ").split(" ");
        words.removeAll("");
        if( words.isEmpty() ) continue;
        line = words.first();
        if( line.isEmpty()
         || line.startsWith(";")
         || line.startsWith(".") ) continue;
        if( line.contains(":") )
        {
            if( words.size() > 1 ) line = words.at(1);
            else continue;
        }
        for( QString instruction : m_avrInstr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) avr++; }
        for( QString instruction : m_picInstr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) pic++; }
        for( QString instruction : m_i51Instr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) i51++; }
        if( ++nlines > 200 ) break;
    }
    int result = 0;
    pic = nlines/pic; avr = nlines/avr; i51 = nlines/i51;

    if     ( pic < 3 && pic < avr && pic < i51 ) result = 1;
    else if( avr < 3 && avr < pic && avr < i51 ) result = 2;
    else if( i51 < 3 && i51 < pic && i51 < avr ) result = 3;

    return result;
}

bool CodeEditor::compile( bool debug )
{
    if( document()->isModified() ){
        if( !EditorWindow::self()->save() )
        {
            m_outPane->appendLine( "Error: File not saved" );
            return false;
    }   }
    m_debugLine  = -1;
    update();
    
    m_outPane->appendLine( "-------------------------------------------------------" );
    int error = m_compiler->compile( debug );

    if( error == 0 ) return true;
    if( error > 0 ) // goto error line number
    {
        m_debugLine = error; // Show arrow in error line
        updateScreen();
    }
    return false;
}

void CodeEditor::addBreakPoint( int line )
{
    if( EditorWindow::self()->debugState() == DBG_RUNNING ) return;
    if( EditorWindow::self()->debugState() > DBG_STOPPED )
        line = m_compiler->getValidLine( line );
    if( !m_brkPoints.contains( line ) ) m_brkPoints.append( line );
}

void CodeEditor::startDebug()
{
    setReadOnly( true );
    QList<int> brkPoints;
    for( int line : m_brkPoints )
    {
        line = EditorWindow::self()->debugger()->getValidLine( line );
        if( !brkPoints.contains( line ) ) brkPoints.append( line );
    }
    m_brkPoints = brkPoints;
    update();
}

void CodeEditor::updateScreen()
{
    setTextCursor( QTextCursor(document()->findBlockByLineNumber( m_debugLine-1 )));
    ensureCursorVisible();
    update();
}

void CodeEditor::readSettings() // Static
{
    m_sintaxPath  = MainWindow::self()->getFilePath("data/codeeditor/sintax/");
    m_compilsPath = MainWindow::self()->getFilePath("data/codeeditor/compilers/");

    m_font.setFamily("Ubuntu Mono");
    m_font.setWeight( 50 );
    //m_font.setFixedPitch( true );
    m_font.setPixelSize( m_fontSize );

    QSettings* settings = MainWindow::self()->settings();

    if( settings->contains( "Editor_show_spaces" ) )
        setShowSpaces( settings->value( "Editor_show_spaces" ).toBool() );

    if( settings->contains( "Editor_tab_size" ) )
        setTabSize( settings->value( "Editor_tab_size" ).toInt() );
    else setTabSize( 4 );

    if( settings->contains( "Editor_font_size" ) )
        setFontSize( settings->value( "Editor_font_size" ).toInt() );

    bool spacesTab = false;
    if( settings->contains( "Editor_spaces_tabs" ) )
        spacesTab = settings->value( "Editor_spaces_tabs" ).toBool();

    setSpaceTabs( spacesTab );
}

void CodeEditor::setFontSize( int size )
{
    m_fontSize = size;
    m_font.setPixelSize( size );
    MainWindow::self()->settings()->setValue( "Editor_font_size", QString::number(m_fontSize) );

    for( CodeEditor* doc : m_documents )
    {
        doc->setFont( m_font );
        doc->setTabSize( m_tabSize );
}   }

void CodeEditor::setTabSize( int size )
{
    m_tabSize = size;
    MainWindow::self()->settings()->setValue( "Editor_tab_size", QString::number(m_tabSize) );

    for( CodeEditor* doc : m_documents )
    {
        doc->setTabStopWidth( m_tabSize*m_fontSize*2/3 );
        if( m_spaceTabs ) doc->setSpaceTabs( true );
}   }

void CodeEditor::setShowSpaces( bool on )
{
    m_showSpaces = on;

    for( CodeEditor* doc : m_documents )
    {
        QTextOption option = doc->document()->defaultTextOption();

        if( on ) option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
        else     option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);

        doc->document()->setDefaultTextOption(option);
    }
    if( m_showSpaces )
         MainWindow::self()->settings()->setValue( "Editor_show_spaces", "true" );
    else MainWindow::self()->settings()->setValue( "Editor_show_spaces", "false" );
}

void CodeEditor::setSpaceTabs( bool on )
{
    m_spaceTabs = on;

    if( on ) { m_tab = ""; for( int i=0; i<m_tabSize; i++) m_tab += " "; }
    else m_tab = "\t";

    if( m_spaceTabs )
         MainWindow::self()->settings()->setValue( "Editor_spaces_tabs", "true" );
    else MainWindow::self()->settings()->setValue( "Editor_spaces_tabs", "false" );
}

void CodeEditor::keyPressEvent( QKeyEvent* event )
{
    if( event->key() == Qt::Key_Plus && (event->modifiers() & Qt::ControlModifier) )
    {
        setFontSize( m_fontSize+1 );
    }
    else if( event->key() == Qt::Key_Minus && (event->modifiers() & Qt::ControlModifier) )
    {
        setFontSize( m_fontSize-1 );
    }
    else if( event->key() == Qt::Key_Tab )
    {
        if( textCursor().hasSelection() ) indentSelection( false );
        else                              insertPlainText( m_tab );
    }
    else if( event->key() == Qt::Key_Backtab )
    {
        if( textCursor().hasSelection() ) indentSelection( true );
        else textCursor().movePosition( QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor , m_tab.size() );
    }
    else{
        int tabs = 0;
        if( event->key() == Qt::Key_Return )
        {
            int n0 = 0;
            int n = m_tab.size();
            QString line = textCursor().block().text();
            
            while(1)
            {
                QString part = line.mid( n0, n );
                if( part == m_tab ) { n0 += n; tabs += 1; }
                else break;
        }   }
        QPlainTextEdit::keyPressEvent( event );
        
        if( event->key() == Qt::Key_Return )
            for( int i=0; i<tabs; i++ ) insertPlainText( m_tab );
}   }

void CodeEditor::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();

    QAction* reloadAction = menu->addAction( QIcon(":/reload.png"), tr("Reload Document") );
    connect( reloadAction, SIGNAL( triggered()),
             EditorWindow::self(), SLOT(reload()), Qt::UniqueConnection );

    menu->exec( event->globalPos() );
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax( 1, blockCount() );
    while( max >= 10 ) { max /= 10; ++digits; }
    return  fontMetrics().height() + fontMetrics().width( QLatin1Char( '9' ) ) * digits;
}

void CodeEditor::updateLineNumberArea( const QRect &rect, int dy )
{
    if( dy ) m_lNumArea->scroll( 0, dy );
    else     m_lNumArea->update( 0, rect.y(), m_lNumArea->width(), rect.height() );
    if( rect.contains( viewport()->rect() ) ) updateLineNumberAreaWidth( 0 );
}

void CodeEditor::resizeEvent( QResizeEvent* e )
{
    QPlainTextEdit::resizeEvent( e );
    QRect cr = contentsRect();
    m_lNumArea->setGeometry( QRect( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if( !isReadOnly() )
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor( 250, 240, 220 );

        selection.format.setBackground( lineColor );
        selection.format.setProperty( QTextFormat::FullWidthSelection, true );
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append( selection );
    }
    setExtraSelections( extraSelections );
}

void CodeEditor::lineNumberAreaPaintEvent( QPaintEvent* event )
{
    QPainter painter( m_lNumArea );
    painter.fillRect( event->rect(), Qt::lightGray );

    QTextBlock block = firstVisibleBlock();

    int blockNumber = block.blockNumber();
    int top       = (int)blockBoundingGeometry( block ).translated( contentOffset() ).top();
    int fontSize  = fontMetrics().height();

    while( block.isValid() && top <= event->rect().bottom() )
    {
        int blockSize = (int)blockBoundingRect( block ).height();
        int bottom = top + blockSize;

        if( block.isVisible() && bottom >= event->rect().top() )
        {
            int lineNumber = blockNumber + 1;
            // Check if there is a new breakpoint request from context menu
            int pos = m_lNumArea->lastPos;
            if( pos > top && pos < bottom)
            {
                if     ( m_brkAction == 1 ) addBreakPoint( lineNumber );
                else if( m_brkAction == 2 ) remBreakPoint( lineNumber );
                else if( m_brkAction == 3 ){
                    if( m_brkPoints.contains( lineNumber ) ) remBreakPoint( lineNumber );
                    else                                     addBreakPoint( lineNumber );
                }
                m_brkAction = 0;
                m_lNumArea->lastPos = 0;
            }
            if( m_brkPoints.contains( lineNumber ) ) // Draw breakPoint icon
            {
                painter.setBrush( QColor(Qt::yellow) );
                painter.setPen( Qt::NoPen );
                painter.drawRect( 0, top, fontSize, fontSize );
            }
            if( lineNumber == m_debugLine ) // Draw debug line icon
                painter.drawImage( QRectF(0, top, fontSize, fontSize), QImage(":/finish.png") );
            // Draw line number
            QString number = QString::number( lineNumber );
            painter.setPen( Qt::black );
            painter.drawText( 0, top, m_lNumArea->width(), fontSize, Qt::AlignRight, number );
        }
        block = block.next();
        top = bottom;
        ++blockNumber;
}   }

void CodeEditor::indentSelection( bool unIndent )
{
    QTextCursor cur = textCursor();
    int a = cur.anchor();
    int p = cur.position();
    
    cur.beginEditBlock();
     
    if( a > p ) std::swap( a, p );
    
    QString str = cur.selection().toPlainText();
    QString str2 = "";
    QStringList list = str.split("\n");
    
    int lines = list.count();
 
    for( int i=0; i<lines; ++i )
    {
        QString line = list[i];

        if( unIndent ) 
        {
            int n = m_tab.size();
            int n1 = n;
            int n2 = 0;
            
            while( n1 > 0 )
            {
                if( line.size() <= n2 ) break;
                QString car = line.at(n2);
                
                if     ( car == " " ) { n1 -= 1; n2 += 1; }
                else if( car == "\t" ) { n1 -= n; if( n1 >= 0 ) n2 += 1; }
                else n1 = 0;
            }
            line.replace( 0, n2, "" );
        }
        else line.insert( 0, m_tab );
        
        if( i < lines-1 ) line += "\n";
        str2 += line;
    }
    cur.removeSelectedText();
    cur.insertText(str2);
    p = cur.position();

    cur.setPosition( a );
    cur.setPosition( p, QTextCursor::KeepAnchor );

    setTextCursor(cur);
    cur.endEditBlock();
}


// ********************* CLASS LineNumberArea **********************************

LineNumberArea::LineNumberArea( CodeEditor *editor )
              : QWidget(editor)
{
    m_codeEditor = editor;
    lastPos = 0;
}
LineNumberArea::~LineNumberArea(){}

void LineNumberArea::contextMenuEvent( QContextMenuEvent *event)
{
    event->accept();
    
    QMenu menu;

    QAction* addBrkAction = menu.addAction( QIcon(":/breakpoint.png"),tr( "Add BreakPoint" ) );
    connect( addBrkAction, SIGNAL( triggered()),
               m_codeEditor, SLOT(slotAddBreak()), Qt::UniqueConnection );

    QAction* remBrkAction = menu.addAction( QIcon(":/nobreakpoint.png"),tr( "Remove BreakPoint" ) );
    connect( remBrkAction, SIGNAL( triggered()),
               m_codeEditor, SLOT(slotRemBreak()), Qt::UniqueConnection );

    menu.addSeparator();

    QAction* clrBrkAction = menu.addAction( QIcon(":/remove.png"),tr( "Clear All BreakPoints" ) );
    connect( clrBrkAction, SIGNAL( triggered()),
               m_codeEditor, SLOT(slotClearBreak()), Qt::UniqueConnection );

    if( menu.exec(event->globalPos()) != 0 ) lastPos = event->pos().y();
}

void LineNumberArea::mousePressEvent( QMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        lastPos = event->pos().y();
        m_codeEditor->toggleBreak();
        this->repaint();
    }
}

#include "moc_codeeditor.cpp"
