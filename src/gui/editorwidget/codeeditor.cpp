/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QTextDocumentFragment>
#include <QSettings>
#include <QDebug>

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
    
    m_compiler = EditorWindow::self()->createDebugger( "", this, "00" );
    m_debugLine = 0;
    m_brkAction = 0;
    m_help = "";

    setFont( m_font );
    setFontSize( m_fontSize );
    setTabSize( m_tabSize );
    setShowSpaces( m_showSpaces );
    setSpaceTabs( m_spaceTabs );
    setAcceptDrops( false );

    QPalette p = palette();
    p.setColor( QPalette::Base, QColor( 255, 255, 249) );
    p.setColor( QPalette::Text, QColor( 0, 0, 0) );
    setPalette( p );

    connect( this, &CodeEditor::blockCountChanged,
             this, &CodeEditor::updateLineNumberAreaWidth, Qt::UniqueConnection );

    connect( this, &CodeEditor::updateRequest,
             this, &CodeEditor::updateLineNumberArea, Qt::UniqueConnection);

    connect( this, &CodeEditor::cursorPositionChanged,
             this, &CodeEditor::highlightCurrentLine, Qt::UniqueConnection);
    
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

void CodeEditor::setSyntaxFile( QString file )
{
    m_hlighter->readSyntaxFile( file );
}

void CodeEditor::setFile( const QString filePath )
{
    if( m_file == filePath ) return;

    m_numLines = document()->blockCount();

    if( m_compiler ) delete m_compiler;
    m_compiler = NULL;

    m_outPane->appendLine( "-------------------------------------------------------" );
    m_outPane->appendLine( tr(" File: ")+filePath+"\n" );

    m_file = filePath;
    QDir::setCurrent( m_file );

    QString extension = getFileExt( filePath );

    QString compiler = changeCompilerFromCode();

    QString code = "00";

    if( extension == ".gcb" )
    {
        m_hlighter->readSyntaxFile("gcbasic.syntax");
        if( !m_compiler ) m_compiler = EditorWindow::self()->createDebugger( "GcBasic", this );
    }
    else if( extension == ".cpp"
          || extension == ".c"
          || extension == ".ino"
          || extension == ".h"
          || extension == ".as" )
    {
        m_hlighter->readSyntaxFile("cpp.syntax");
        if( extension == ".ino" )
        {   if( !m_compiler ) m_compiler = EditorWindow::self()->createDebugger( "Arduino", this );}
        else if( extension == ".as" )
        {   if( !m_compiler ) m_compiler = EditorWindow::self()->createDebugger( "AScript", this );}
        else if( extension == ".cpp" || extension == ".c") code = "10";
    }
    /*else if( extension == ".s" )
    {
        m_hlighter->readSyntaxFile( "avrasm.syntax" );
        m_compiler = EditorWindow::self()->createDebugger( "Avrgcc-asm", this );
    }*/
    else if( extension == ".a51" ) // 8051
    {
        m_outPane->appendLine( "I51 asm\n" );
        m_hlighter->readSyntaxFile("i51asm.syntax");
    }
    else if( extension == ".asm" ) // We should identify if pic, avr or i51 asm
    {
        m_outPane->appendText( tr("File recognized as: ") );

        int type = getSyntaxCoincidences();
        if( type == 1 )   // Is Pic
        {
            m_outPane->appendLine( "Pic asm\n" );
            m_hlighter->readSyntaxFile("pic14asm.syntax");
        }
        else if( type == 2 )  // Is Avr
        {
            m_outPane->appendLine( "Avr asm\n" );
            m_hlighter->readSyntaxFile("avrasm.syntax");
        }
        else if( type == 3 )  // Is 8051
        {
            m_outPane->appendLine( "I51 asm\n" );
            m_hlighter->readSyntaxFile("i51asm.syntax");
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
        m_hlighter->readSyntaxFile("xml.syntax");
    }
    else if( getFileName( m_file ).toLower() == "makefile"  )
    {
        m_hlighter->readSyntaxFile("makef.syntax");
    }
    else if( extension == ".hex"
         ||  extension == ".ihx" )
    {
        m_font.setLetterSpacing( QFont::PercentageSpacing, 110 );
        setFont( m_font );
        m_hlighter->readSyntaxFile("hex.syntax");
    }
    else if( extension == ".js" )
    {
        m_hlighter->readSyntaxFile("js.syntax");
    }
    /*else if( extension == ".sac" )
    {
        //m_compiler = new B16AsmDebugger( this, m_outPane );
    }*/
    if( !m_compiler ) m_compiler = EditorWindow::self()->createDebugger( compiler, this, code );
    m_outPane->appendLine( "-------------------------------------------------------" );
}

int CodeEditor::getSyntaxCoincidences()
{
    QStringList lines = fileToStringList( m_file, "CodeEditor::getSintaxCoincidences" );

    double avr=1, pic=1, i51=1; // Avoid divide by 0
    int nlines = 0;
    int matches = 0;

    for( QString line : lines )
    {
        QStringList words = line.toLower().replace("\t", " ").split(" ");
        words.removeAll("");
        if( words.isEmpty() ) continue;
        line = words.first();
        if( line.contains(":") )
        {
            if( words.size() > 1 ) line = words.at(1);
            else continue;
        }
        if( line.isEmpty()
         || line.startsWith("#")
         || line.startsWith(";")
         || line.startsWith(".") ) continue;

        for( QString instruction : m_avrInstr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) {avr++; matches++;} }
        for( QString instruction : m_picInstr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) {pic++; matches++;} }
        for( QString instruction : m_i51Instr )
        { if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) )) {i51++; matches++;} }
        if( matches > 200 || ++nlines > 400 ) break;
    }
    if( matches == 0 ) return 0;
    int result = 0;
    pic = matches/pic; avr = matches/avr; i51 = matches/i51;

    if     ( pic < 3 && pic < avr && pic < i51 ) result = 1;
    else if( avr < 3 && avr < pic && avr < i51 ) result = 2;
    else if( i51 < 3 && i51 < pic && i51 < avr ) result = 3;

    return result;
}

bool CodeEditor::compile( bool debug )
{
    if( document()->isModified() )
    {
        changeCompilerFromCode();

        if( !EditorWindow::self()->save() )
        {
            m_outPane->appendLine( "Error: File not saved" );
            return false;
    }   }
    m_outPane->appendLine( "-------------------------------------------------------" );
    m_errors.clear();
    m_warnings.clear();
    int error = m_compiler->compile( debug );
    update();

    if( error == 0 ) return true;
    if( error > 0 ) // goto error line number
    {
        setTextCursor( QTextCursor(document()->findBlockByNumber( error-1 )));
        ensureCursorVisible();
    }
    else m_outPane->appendLine( "\n"+tr("     WARNING: Compilation Not Done")+"\n" );
    return false;
}

void CodeEditor::addBreakPoint( int line )
{
    if( EditorWindow::self()->debugState() == DBG_RUNNING ) return;
    if( EditorWindow::self()->debugState() > DBG_STOPPED )
        line = EditorWindow::self()->debugger()->getValidLine( {m_file, line} );
    if( line > 0 && !m_brkPoints.contains( line ) )
    {
        //QTextBlock block = document()->findBlockByNumber( line-1 );
        //UserData* data = (UserData*)block.userData();
        //if( !data ) data = new UserData();
        //data->breakp = true;
        //block.setUserData( data );

        m_brkPoints.append( line );
        update();
    }
}

void CodeEditor::remBreakPoint( int line )
{
    m_brkPoints.removeOne( line );
    //QTextBlock block = document()->findBlockByNumber( line-1 );
    //UserData* data = (UserData*)block.userData();
    //data->breakp = false;
    update();
}

void CodeEditor::startDebug()
{
    setReadOnly( true );
    QList<int> brkPoints;
    for( int line : m_brkPoints )
    {
        line = EditorWindow::self()->debugger()->getValidLine( {m_file, line} );
        if( line > 0 && !brkPoints.contains( line ) ) brkPoints.append( line );
    }
    m_brkPoints = brkPoints;
    update();
}

void CodeEditor::stopDebug()
{
    setReadOnly( false );
    setDebugLine( 0 );
    update();
}

void CodeEditor::showMsg( QString msg )
{
    m_outPane->appendLine( msg+"\n" );
}

void CodeEditor::updateScreen()
{
    setTextCursor( QTextCursor(document()->findBlockByLineNumber( m_debugLine-1 )));
    ensureCursorVisible();
    update();
}

void CodeEditor::readSettings() // Static
{
    m_font.setFamily("Ubuntu Mono");
    m_font.setWeight( 50 );
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
    else if( event->key() == Qt::Key_F && (event->modifiers() & Qt::ControlModifier) )
    {
        EditorWindow::self()->findReplaceDialog();
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
    }else{
        QString tabs;
        if( event->key() == Qt::Key_Return )
        {
            int n0 = 0;
            int n = m_tab.size();
            QString line = textCursor().block().text();

            while(1)
            {
                QString part = line.mid( n0, n );
                if( part == m_tab ) { n0 += n; tabs += m_tab; }
                else break;
        }   }
        QPlainTextEdit::keyPressEvent( event );

        if( event->key() == Qt::Key_Return ) insertPlainText( tabs );
}   }

void CodeEditor::deleteSelected()
{
    textCursor().insertText( "" );
}

void CodeEditor::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu;

    QAction* undoAction = menu.addAction(QIcon(":/undo.svg"),tr("Undo")+"\tCtrl+Z");
    connect( undoAction, &QAction::triggered,
              this, &CodeEditor::undo, Qt::UniqueConnection );

    QAction* redoAction = menu.addAction(QIcon(":/redo.svg"),tr("Redo")+"\tCtrl+Y");
    connect( redoAction, &QAction::triggered,
              this, &CodeEditor::redo, Qt::UniqueConnection );

    menu.addSeparator();

    QAction* cutAction = menu.addAction(QIcon(":/cut.svg"),tr("Cut")+"\tCtrl+X");
    connect( cutAction, &QAction::triggered,
                  this, &CodeEditor::cut, Qt::UniqueConnection );

    QAction* copyAction = menu.addAction(QIcon(":/copy.svg"),tr("Copy")+"\tCtrl+C");
    connect( copyAction, &QAction::triggered,
                   this, &CodeEditor::copy, Qt::UniqueConnection );

    QAction* pasteAction = menu.addAction(QIcon(":/paste.svg"),tr("Paste")+"\tCtrl+V");
    connect( pasteAction, &QAction::triggered,
                    this, &CodeEditor::paste, Qt::UniqueConnection );

    QAction* removeAction = menu.addAction( QIcon( ":/remove.svg"),tr("Remove") );
    connect( removeAction, &QAction::triggered,
                     this, &CodeEditor::deleteSelected, Qt::UniqueConnection );

    if( !textCursor().hasSelection() ) removeAction->setDisabled( true );

    menu.addSeparator();

    QAction* reloadAction = menu.addAction(QIcon(":/reload.svg"), tr("Reload Document")+"\tCtrl+R");
    connect( reloadAction, &QAction::triggered,
             EditorWindow::self(), &EditorWindow::reload, Qt::UniqueConnection );

    menu.exec( event->globalPos() );
}

QString CodeEditor::changeCompilerFromCode()
{
    QString line = this->document()->findBlockByLineNumber(0).text();
    QString compiler = BaseDebugger::getValue( line, "compiler" );
    if( compiler.isEmpty() ) compiler = "None";
    else{
        m_outPane->appendLine( tr("Found Compiler definition in file: ") + compiler );
        if( m_compiler != NULL )
        {
            if( m_compiler->compName()==compiler ) return compiler;
            delete m_compiler;
        }
        m_compiler = EditorWindow::self()->createDebugger( compiler, this );
        m_compiler->getInfoInFile( line );
    }
    return compiler;
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

QList<int> CodeEditor::getFound()
{
    QList<int> found;
    for( QTextEdit::ExtraSelection es : m_found )
        found.append( es.cursor.blockNumber() );
    return found;
}

void CodeEditor::setFound( QList<QTextEdit::ExtraSelection> sel )
{
    m_found = sel;
    //update();
    highlightCurrentLine();
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if( m_found.size() ) extraSelections.append( m_found );

    if( !isReadOnly() )
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor( 250, 240, 220 );

        selection.format.setBackground( lineColor );
        selection.format.setProperty( QTextFormat::FullWidthSelection, true );
        selection.cursor = textCursor();
        extraSelections.prepend( selection );
    }
    setExtraSelections( extraSelections );
}

void CodeEditor::lineNumberAreaPaintEvent( QPaintEvent* event )
{
    QPainter painter( m_lNumArea );
    painter.fillRect( event->rect(), Qt::lightGray );

    QTextBlock block = firstVisibleBlock();

    int top       = (int)blockBoundingGeometry( block ).translated( contentOffset() ).top();
    int fontSize  = fontMetrics().height();

    int numLines = document()->blockCount();
    if( m_numLines != numLines )                 // Line added or removed: move or remove points
    {
        int delta = m_numLines - numLines;
        m_numLines = numLines;
        bool found = false;
        QTextBlock cBlock = textCursor().block().next();

        QList<int> errors    = m_errors;  // Copy lists to make substitutions
        QList<int> warnings  = m_warnings;
        QList<int> brkPoints = m_brkPoints;

        while( block.isValid() )
        {
            int newLine = block.blockNumber() + 1;
            int oldLine = newLine + delta;
            if( block == cBlock )
            {
                if( delta > 0 ) // Line removed, check if point at cursor line and remove it
                {
                    if( m_brkPoints.contains( newLine ) ) brkPoints.removeOne( newLine );
                    if( m_errors.contains( newLine )    ) errors.removeOne( newLine );
                    if( m_warnings.contains( newLine )  ) warnings.removeOne( newLine );
                }
                found = true;
            }
            if( found ) // Replace lines
            {
                if( m_brkPoints.contains( oldLine ) ) brkPoints.replace( m_brkPoints.indexOf( oldLine ), newLine ); // Replace breakpoint line
                if( m_errors.contains( oldLine )    ) errors.replace(    m_errors.indexOf( oldLine )   , newLine ); // Replace error line
                if( m_warnings.contains( oldLine )  ) warnings.replace(  m_warnings.indexOf( oldLine ) , newLine ); // Replace warning line
            }
            block = block.next();
        }
        m_errors    = errors;      // Replace old lists with new ones
        m_warnings  = warnings;
        m_brkPoints = brkPoints;

        block = firstVisibleBlock();
    }
    while( block.isValid()  && top <= event->rect().bottom() )
    {
        int blockSize = (int)blockBoundingRect( block ).height();
        int bottom = top + blockSize;
        int lineNumber = block.blockNumber() + 1;

        if( block.isVisible() && bottom >= event->rect().top() )
        {
            BaseDebugger* bd = EditorWindow::self()->debugger();
            if( bd && bd->isMappedLine( {m_file,lineNumber} ) )
            {
                painter.setBrush( QColor( 180, 180, 170 ) );
                painter.setPen( Qt::NoPen );
                painter.drawRect( 0, top, fontSize, fontSize );
            }
            if( m_compiler )                            // Breakpoints an error indicator
            {
                painter.setRenderHint( QPainter::Antialiasing );
                //UserData* data = (UserData*)block.userData();

                int pos = m_lNumArea->lastPos;
                if( pos > top && pos < bottom ) // Check if there is a new breakpoint request from context menu
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

                if( m_brkPoints.contains( lineNumber ) ) // Draw breakPoint icon //( data && data->breakp ) //
                {
                    painter.setOpacity( 0.5 );
                    painter.fillRect( QRectF( 0, top+1, fontSize, fontSize-2 ), QColor(50, 50, 0) );
                    painter.setOpacity( 1 );
                    painter.fillRect( QRectF( 1, top+2, fontSize-2, fontSize-4 ), Qt::yellow );
                }

                if( lineNumber == m_debugLine ) // Draw debug line icon
                    painter.drawImage( QRectF( 0, top, fontSize, fontSize ), QImage(":/brkpoint.png") );

                if( m_warnings.contains( lineNumber ) ) // Draw warning
                {
                    painter.setBrush( QColor(220, 120, 0) ); // Orange
                    painter.setPen( Qt::NoPen );
                    QPolygon polygon;
                    polygon << QPoint(1,bottom-2) << QPoint(fontSize/2,top+1) << QPoint(fontSize-1,bottom-2);
                    painter.drawPolygon( polygon );
                    painter.setPen( Qt::white );
                    painter.drawText( 3, top+4, fontSize-6, fontSize-6, Qt::AlignCenter, "!");
                }
                if( m_errors.contains( lineNumber ) ) // Draw error
                {
                    painter.setBrush( QColor(Qt::red) );
                    painter.setPen( Qt::NoPen );
                    painter.drawEllipse( 2, top+2, fontSize-4, fontSize-4 );
                    painter.setPen( Qt::white );
                    painter.drawText( 3, top+3, fontSize-6, fontSize-6, Qt::AlignCenter, "!");
                }
            }
            QString number = QString::number( lineNumber ); // Draw line number
            painter.setPen( Qt::black );
            painter.drawText( 0, top, m_lNumArea->width(), fontSize, Qt::AlignRight, number );
        }
        block = block.next();
        top = bottom;
    }
}

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
    connect( addBrkAction, &QAction::triggered,
               m_codeEditor, &CodeEditor::slotAddBreak, Qt::UniqueConnection );

    QAction* remBrkAction = menu.addAction( QIcon(":/nobreakpoint.png"),tr( "Remove BreakPoint" ) );
    connect( remBrkAction, &QAction::triggered,
               m_codeEditor, &CodeEditor::slotRemBreak, Qt::UniqueConnection );

    menu.addSeparator();

    QAction* clrBrkAction = menu.addAction( QIcon(":/remove.svg"),tr( "Clear All BreakPoints" ) );
    connect( clrBrkAction, &QAction::triggered,
               m_codeEditor, &CodeEditor::slotClearBreak, Qt::UniqueConnection );

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
