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

#include <QtGui>
#include <QThread>
#include <QtAlgorithms>
#include <QRegExp>

#include "codeeditor.h"
#include "baseprocessor.h"
#include "gcbdebugger.h"
#include "inodebugger.h"
#include "b16asmdebugger.h"
#include "avrasmdebugger.h"
#include "picasmdebugger.h"
#include "mcucomponent.h"
#include "mainwindow.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "editorwindow.h"
#include "editorprop.h"
#include "simuapi_apppath.h"
#include "utils.h"

static const char* CodeEditor_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Font Size"),
    QT_TRANSLATE_NOOP("App::Property","Tab Size"),
    QT_TRANSLATE_NOOP("App::Property","Spaces Tabs"),
    QT_TRANSLATE_NOOP("App::Property","Show Spaces")
};

QStringList CodeEditor::m_picInstr = QString("addlw addwf andlw andwf bcf bov bsf btfsc btg btfss clrf clrw clrwdt comf decf decfsz goto incf incfsz iorlw iorwf movf movlw movwf reset retfie retlw return rlf rrfsublw subwf swapf xorlw xorwf")
                .split(" ");
QStringList CodeEditor::m_avrInstr = QString("add adc adiw sub subi sbc sbci sbiw andi ori eor com neg sbr cbr dec tst clr ser mul rjmp ijmp jmp rcall icall ret reti cpse cp cpc cpi sbrc sbrs sbic sbis brbs brbc breq brne brcs brcc brsh brlo brmi brpl brge brlt brhs brhc brts brtc brvs brvc brie brid mov movw ldi lds ld ldd sts st std lpm in out push pop lsl lsr rol ror asr swap bset bclr sbi cbi bst bld sec clc sen cln sez clz sei cli ses cls sev clv set clt seh clh wdr")
                .split(" ");
 
bool  CodeEditor::m_showSpaces = false;
bool  CodeEditor::m_spaceTabs  = false;
bool  CodeEditor::m_driveCirc  = false;
int   CodeEditor::m_fontSize = 13;
int   CodeEditor::m_tabSize = 4;
QFont CodeEditor::m_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

CodeEditor::CodeEditor( QWidget* parent, OutPanelText* outPane )
          : QPlainTextEdit( parent )
          , Updatable()
{
    Q_UNUSED( CodeEditor_properties );
    
    setObjectName( "Editor" );
    
    m_outPane   = outPane;
    m_lNumArea  = new LineNumberArea( this );
    m_hlighter  = new Highlighter( document() );
    
    m_debugger = NULL;
    m_debugLine = 0;
    m_brkAction = 0;

    m_isCompiled= false;
    m_driveCirc = false;
    m_propDialog = NULL;

    m_help = "";
    m_state = DBG_STOPPED;

    m_font.setFamily("Monospace");
    m_font.setFixedPitch( true );
    m_font.setPixelSize( m_fontSize );
    setFont( m_font );
    
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
    Simulator::self()->remFromUpdateList( this );

    if( !m_propDialog ) return;

    m_propDialog->setParent( NULL );
    m_propDialog->close();
    m_propDialog->deleteLater();
}

void CodeEditor::setFile( const QString filePath )
{
    m_isCompiled= false;
    if( m_file == filePath ) return;

    if( m_propDialog )
    {
        m_propDialog->setParent( NULL );
        m_propDialog->close();
        m_propDialog->deleteLater();
        m_propDialog = NULL;
    }
    if( m_debugger )
    {
        delete m_debugger;
        m_debugger = NULL;
    }
    
    m_outPane->appendLine( "-------------------------------------------------------" );
    m_outPane->appendLine( tr(" File: ")+filePath+"\n" );

    m_file = filePath;
    QFileInfo fi = QFileInfo( m_file );

    m_fileDir  = fi.absolutePath();
    m_fileExt  = fi.suffix();
    m_fileName = fi.completeBaseName();

    QDir::setCurrent( m_file );

    QString sintaxPath = SIMUAPI_AppPath::self()->availableDataFilePath("codeeditor/sintax/");
    QString compilerPath = SIMUAPI_AppPath::self()->availableDataFilePath("codeeditor/compilers/");

    if( m_fileExt == "gcb" )
    {
        m_hlighter->readSintaxFile( sintaxPath + "gcbasic.sintax" );

        m_debugger = new GcbDebugger( this, m_outPane, filePath );
        m_debugger->loadCompiler( compilerPath+"gcbcompiler.xml" );
    }
    else if( m_fileExt == "cpp"
          || m_fileExt == "c"
          || m_fileExt == "ino"
          || m_fileExt == "h" )
    {
        m_hlighter->setMultiline( true );
        m_hlighter->readSintaxFile( sintaxPath + "cpp.sintax" );
        
        if( m_fileExt == "ino" ) m_debugger = new InoDebugger( this, m_outPane, filePath );
    }
    else if( m_fileExt == "asm" ) // We should identify if pic or avr asm
    {
        int isPic = 0;
        int isAvr = 0;
        
        isPic = getSintaxCoincidences( m_file, m_picInstr );
        
        if( isPic < 50 ) isAvr = getSintaxCoincidences( m_file, m_avrInstr );

        m_outPane->appendLine( tr("File recognized as: ") );

        if( isPic > isAvr )   // Is Pic
        {
            m_outPane->appendLine( "Pic asm\n" );

            QString path = sintaxPath + "pic14asm.sintax";
            m_hlighter->readSintaxFile( path );

            m_debugger = new PicAsmDebugger( this, m_outPane, filePath );
        }
        else if( isAvr > isPic )  // Is Avr
        {
            m_outPane->appendLine( "Avr asm\n" );

            QString path = sintaxPath + "avrasm.sintax";
            m_hlighter->readSintaxFile( path );

            m_debugger = new AvrAsmDebugger( this, m_outPane, filePath );
            m_debugger->loadCompiler( compilerPath+"avracompiler.xml" );
        }
        else m_outPane->appendLine( "Unknown\n" );
    }
    else if( m_fileExt == "xml"
         ||  m_fileExt == "package"
         ||  m_fileExt == "simu" )
    {
        m_hlighter->readSintaxFile( sintaxPath + "xml.sintax" );
    }
    else if( m_fileName.toLower() == "makefile"  )
    {
        m_hlighter->readSintaxFile( sintaxPath + "makef.sintax" );
    }
    else if( m_fileExt == "sac" )
    {
        m_debugger = new B16AsmDebugger( this, m_outPane, filePath );
    }
}

int CodeEditor::getSintaxCoincidences( QString& fileName, QStringList& instructions )
{
    QStringList lines = fileToStringList( fileName, "CodeEditor::getSintaxCoincidences" );

    int coincidences = 0;

    for( QString line : lines )
    {
        if( line.isEmpty()      ) continue;
        if( line.startsWith("#")) continue;
        if( line.startsWith(";")) continue;
        if( line.startsWith(".")) continue;
        line =line.toLower();
        
        for( QString instruction : instructions )
        {
            if( line.contains( QRegExp( "\\b"+instruction+"\\b" ) ))
                coincidences++;
            
            if( coincidences > 50 ) break;
        }
    }
    return coincidences;
}

void CodeEditor::setCompilerPath()
{
    if( m_debugger ) m_debugger->getCompilerPath();
    else
    {
        if( m_fileExt == "" ) MessageBoxNB( "CodeEditor::setCompilerPath",
                                        tr( "Please save the Document first" ) );
        else MessageBoxNB( "CodeEditor::setCompilerPath",
                       tr( "No Compiler available for: %1 files" ).arg(m_fileExt));
    }
}

void CodeEditor::compile()
{
    if( document()->isModified() ) EditorWindow::self()->save();
    m_debugLine  = -1;
    update();
    
    int error=-2;
    m_isCompiled = false;
    
    m_outPane->appendLine( "-------------------------------------------------------" );
    m_outPane->appendText( "Exec: ");

    if( m_fileName.toLower() == "makefile" )          // Is a Makefile, make it
    {
        m_outPane->appendLine( "make "+m_file+"\n" );

        QProcess makeproc( 0l );
        makeproc.setWorkingDirectory( m_fileDir );
        makeproc.start( "make" );
        makeproc.waitForFinished(-1);

        QString p_stdout = makeproc.readAllStandardOutput();
        QString p_stderr = makeproc.readAllStandardError();
        m_outPane->appendText( p_stderr );
        m_outPane->appendText( "\n" );
        m_outPane->appendLine( p_stdout );

        if( p_stderr.toUpper().contains("ERROR") || p_stdout.toUpper().contains("ERROR") )
            error = -1;
        else error = 0;
    }
    else{
        if( !m_debugger )
        {
            m_outPane->appendLine( "\n"+tr("File type not supported")+"\n" );
            return;
        }
        error = m_debugger->compile();
    }

    if( error == 0 )
    {
        m_outPane->appendLine( "\n"+tr("     SUCCESS!!! Compilation Ok")+"\n" );
        m_isCompiled = true;
    }
    else{
        m_outPane->appendLine( "\n"+tr("     ERROR!!! Compilation Failed")+"\n" );
        
        if( error > 0 ) // goto error line number
        {
            m_debugLine = error; // Show arrow in error line
            updateScreen();
        }
    }
}

void CodeEditor::upload()
{
    if( m_file.endsWith(".hex") )     // is an .hex file, upload to proccessor
    {
        //m_outPane->appendLine( "-------------------------------------------------------\n" );
        m_outPane->appendLine( "\n"+tr("Uploading: ") );
        m_outPane->appendLine( m_file );

        if( McuComponent::self() ) McuComponent::self()->load( m_file );
        return;
    }
    if( !m_isCompiled ) compile();
    if( !m_isCompiled ) return;
    if( m_debugger ) m_debugger->upload();
}

void CodeEditor::addBreakPoint( int line )
{
    if( m_state == DBG_RUNNING ) return;
    
    line = m_debugger->getValidLine( line );
    if( !m_brkPoints.contains( line ) ) m_brkPoints.append( line );
}

void CodeEditor::remBreakPoint( int line ) { m_brkPoints.removeOne( line ); }

bool CodeEditor::initDebbuger()
{
    m_outPane->appendLine( "-------------------------------------------------------\n" );
    m_outPane->appendLine( tr("Starting Debbuger...")+"\n" );

    bool error = false;
    m_state = DBG_STOPPED;
    
    if( !McuComponent::self() )             // Must be an Mcu in Circuit
    {
        m_outPane->appendLine( "\n    "+tr("Error: No Mcu in Simulator... ")+"\n" );
        error = true;
    }
    else if( !m_debugger )             // No debugger for this file type
    {
        m_outPane->appendLine( "\n    "+tr("Error: No Debugger Suited for this File... ")+"\n" );
        error = true;
    }
    else if( m_file == "" )                                   // No File
    {
        m_outPane->appendLine( "\n    "+tr("Error: No File... ")+"\n" );
        error = true;
    }
    else
    {
        compile();
        if( !m_isCompiled )                           // Error compiling
        {
            m_outPane->appendLine( "\n    "+tr("Error Compiling... ")+"\n" );
            error = true;
        }
        else if( !m_debugger->upload() )      // Error Loading Firmware
        {
            m_outPane->appendLine( "\n    "+tr("Error Loading Firmware... ")+"\n" );
            error = true;
        }
    }
    m_outPane->appendLine( "\n" );

    if( error ) stopDebbuger();
    else                                          // OK: Start Debugging
    {
        if( m_debugger->type==1 ) EditorWindow::self()->enableStepOver( true );
        else                      EditorWindow::self()->enableStepOver( false );

        Simulator::self()->addToUpdateList( this );

        BaseProcessor::self()->setDebugging( true );
        reset();
        setDriveCirc( m_driveCirc );
        CircuitWidget::self()->powerCircDebug( m_driveCirc );

        m_outPane->appendLine( tr("Debugger Started ")+"\n" );
        setReadOnly( true );
    }
    return ( m_state == DBG_PAUSED );
}

void CodeEditor::runToBreak()
{
    if( m_state == DBG_STOPPED ) return;

    m_state = DBG_RUNNING;
    if( m_driveCirc ) Simulator::self()->resumeSim();
    BaseProcessor::self()->stepOne( m_debugLine );
}

void CodeEditor::step( bool over )
{
    if( m_state == DBG_RUNNING ) return;

    if( over )
    {
        addBreakPoint( m_debugLine+1 );
        EditorWindow::self()->run();
    }else {
        m_state = DBG_STEPING;
        BaseProcessor::self()->stepOne( m_debugLine );
        if( m_driveCirc ) Simulator::self()->resumeSim();
    }
}

void CodeEditor::stepOver()
{
    QList<int> subLines = m_debugger->getSubLines();
    bool over = false;
    if( subLines.contains( m_debugLine ) ) over = true;

    step( over );
}

void CodeEditor::lineReached( int line ) // Processor reached PC related to source line
{
    m_debugLine = line;

    if( ( m_state == DBG_RUNNING )  // We are running to Breakpoint
     && !m_brkPoints.contains( m_debugLine ) ) // Breakpoint not reached, Keep stepping
    {
        BaseProcessor::self()->stepOne( m_debugLine );
        return;
    }
    EditorWindow::self()->pause(); // EditorWindow: calls this->pause as well

    int cycle = BaseProcessor::self()->cycle();
    m_outPane->appendLine( tr("Clock Cycles: ")+QString::number( cycle-m_lastCycle ));
    m_lastCycle = cycle;
}

void CodeEditor::stopDebbuger()
{
    if( m_state > DBG_STOPPED )
    {
        m_brkPoints.clear();
        m_debugLine = 0;
        
        CircuitWidget::self()->powerCircOff();
        BaseProcessor::self()->setDebugging( false );
        Simulator::self()->remFromUpdateList( this );
        
        m_state = DBG_STOPPED;
        setReadOnly( false );
        updateScreen();
    }
    m_outPane->appendLine( "\n"+tr("Debugger Stopped ")+"\n" );
}

void CodeEditor::pause()
{
    if( m_state < DBG_STEPING )  return;

    if( m_driveCirc ) Simulator::self()->pauseSim();

    m_resume = m_state;
    m_state  = DBG_PAUSED;
    //updateScreen();
}

void CodeEditor::reset()
{
    if( m_state == DBG_RUNNING ) pause();

    McuComponent::self()->reset();
    m_debugLine = 1;
    m_lastCycle = 0;
    m_state = DBG_PAUSED;

    updateScreen();
}

void CodeEditor::setDriveCirc( bool drive )
{
    m_driveCirc = drive;
    
    if( m_state == DBG_PAUSED )
    {
        if( drive ) Simulator::self()->pauseSim();
    }
}

void CodeEditor::updateStep()
{
    if( m_state == DBG_PAUSED ) updateScreen();
}

void CodeEditor::updateScreen()
{
    setTextCursor( QTextCursor(document()->findBlockByLineNumber( m_debugLine-1 )));
    ensureCursorVisible();
    update();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax( 1, blockCount() );
    while( max >= 10 ) { max /= 10; ++digits; }
    return  fontMetrics().height() + fontMetrics().width( QLatin1Char( '9' ) ) * digits;
}

void CodeEditor::updateLineNumberAreaWidth( int /* newBlockCount */ )
{
    setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
}

void CodeEditor::updateLineNumberArea( const QRect &rect, int dy )
{
    if( dy ) m_lNumArea->scroll( 0, dy );
    else     m_lNumArea->update( 0, rect.y(), m_lNumArea->width(), rect.height() );
    if( rect.contains( viewport()->rect() ) ) updateLineNumberAreaWidth( 0 );
}

void CodeEditor::resizeEvent( QResizeEvent *e )
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

void CodeEditor::lineNumberAreaPaintEvent( QPaintEvent *event )
{
    QPainter painter( m_lNumArea );
    painter.fillRect( event->rect(), Qt::lightGray );

    QTextBlock block = firstVisibleBlock();

    int blockNumber = block.blockNumber();
    int top       = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
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
                if      ( m_brkAction == 1 ) addBreakPoint( lineNumber );
                else if ( m_brkAction == 2 ) remBreakPoint( lineNumber );
                m_brkAction = 0;
                m_lNumArea->lastPos = 0;
            }
            // Draw breakPoint icon
            if( m_brkPoints.contains(lineNumber) )
            {
                painter.setBrush( QColor(Qt::yellow) );
                painter.setPen( Qt::NoPen );
                painter.drawRect( 0, top, fontSize, fontSize );
            }
            // Draw debug line icon
            if( lineNumber == m_debugLine )
                painter.drawImage( QRectF(0, top, fontSize, fontSize), QImage(":/finish.png") );
            // Draw line number
            QString number = QString::number( lineNumber );
            painter.setPen( Qt::black );
            painter.drawText( 0, top, m_lNumArea->width(), fontSize, Qt::AlignRight, number );
        }
        block = block.next();
        top = bottom;
        ++blockNumber;
    }
}

/*void CodeEditor::focusInEvent( QFocusEvent* event)
{
    //QPropertyEditorWidget::self()->setObject( this );
    //if( m_debugger ) QPropertyEditorWidget::self()->addObject( m_debugger );
    QPlainTextEdit::focusInEvent( event );
}*/

void CodeEditor::setFontSize( int size )
{
    m_fontSize = size;
    m_font.setPixelSize( size );
    setFont( m_font );
    
    MainWindow::self()->settings()->setValue( "Editor_font_size", QString::number(m_fontSize) );
    
    setTabSize( m_tabSize );
}

void CodeEditor::setTabSize( int size )
{
    m_tabSize = size;
    setTabStopWidth( m_tabSize*m_fontSize*2/3 );
    
    MainWindow::self()->settings()->setValue( "Editor_tab_size", QString::number(m_tabSize) );
    
    if( m_spaceTabs ) setSpaceTabs( true );
}

bool CodeEditor::showSpaces()
{
    return m_showSpaces;
}
void CodeEditor::setShowSpaces( bool on )
{
    m_showSpaces = on;
    
    QTextOption option =  document()->defaultTextOption();
    
    if( on ) option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);

    else option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);

    document()->setDefaultTextOption(option);
    
    if( m_showSpaces )
         MainWindow::self()->settings()->setValue( "Editor_show_spaces", "true" );
    else MainWindow::self()->settings()->setValue( "Editor_show_spaces", "false" );
}

bool CodeEditor::spaceTabs()
{
    return m_spaceTabs;
}

void CodeEditor::setSpaceTabs( bool on )
{
    m_spaceTabs = on;

    if( on )
    {
        m_tab = "";
        for( int i=0; i<m_tabSize; i++) m_tab += " ";
    }
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
        else
        {
            textCursor().movePosition( QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor , m_tab.size() );
        }
    }else{
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
            }
        } 
        QPlainTextEdit::keyPressEvent( event );
        
        if( event->key() == Qt::Key_Return )
            for( int i=0; i<tabs; i++ ) insertPlainText( m_tab );
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *menu = createStandardContextMenu();
    QAction* propertiesAction = menu->addAction( QIcon( ":/properties.png"),tr("Properties") );
    connect( propertiesAction, SIGNAL( triggered()),
                         this, SLOT( slotProperties() ), Qt::UniqueConnection );
    menu->exec(event->globalPos());
}

void CodeEditor::slotProperties()
{
    if( !m_propDialog )
    {
        m_propDialog = new EditorProp( this, m_debugger );
    }
    m_propDialog->show();

    /*if( m_properties ) m_propertiesW->show();
    else
    {
        if( m_help == "" )
        {
            m_help = MainWindow::self()->getHelpFile( "editor" );
        }
        m_propertiesW = MainWindow::self()->createPropWidget( this, &m_help );
        if( m_debugger ) m_propertiesW->properties()->addObject( m_debugger );
        m_propertiesW->setWindowTitle( m_fileName+m_fileExt );

        QPoint p = EditorWindow::self()->mapToGlobal( QPoint(0, 0) );
        m_propertiesW->move( p.x(), p.y() );

        m_properties = true;
    }*/
}

/*void CodeEditor::increaseSelectionIndent()
{
    QTextCursor curs = textCursor();

    // Get the first and count of lines to indent.

    int spos = curs.anchor();
    int epos = curs.position();

    if( spos > epos ) std::swap(spos, epos);

    curs.setPosition( spos, QTextCursor::MoveAnchor );
    int sblock = curs.block().blockNumber();

    curs.setPosition( epos, QTextCursor::MoveAnchor );
    int eblock = curs.block().blockNumber();

    // Do the indent.

    curs.setPosition( spos, QTextCursor::MoveAnchor );

    curs.beginEditBlock();

    for( int i = 0; i <= ( eblock-sblock); ++i )
    {
        curs.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );

        curs.insertText( m_tab );

        curs.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor );
    }
    curs.endEditBlock();

    // Set our cursor's selection to span all of the involved lines.

    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );

    while( curs.block().blockNumber() < eblock )
    {
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor );
    }
    curs.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );

    setTextCursor( curs );
}*/

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
 
    for (int i = 0; i < lines; i++)
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
                
                if( car == " " ) 
                {
                    n1 -= 1;
                    n2 += 1;
                }
                else if( car == "\t" )
                {
                    n1 -= n;
                    if( n1 >= 0 ) n2 += 1;
                }
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



// CLASS LineNumberArea ******************************************************
LineNumberArea::LineNumberArea( CodeEditor *editor ) : QWidget(editor)
{
    m_codeEditor = editor;
}
LineNumberArea::~LineNumberArea(){}

void LineNumberArea::contextMenuEvent( QContextMenuEvent *event)
{
    event->accept();
    
    if( !m_codeEditor->debugStarted() ) return;
    
    QMenu menu;

    QAction *addm_brkAction = menu.addAction( QIcon(":/breakpoint.png"),tr( "Add BreakPoint" ) );
    connect( addm_brkAction, SIGNAL( triggered()),
               m_codeEditor, SLOT(slotAddBreak()), Qt::UniqueConnection );

    QAction *remm_brkAction = menu.addAction( QIcon(":/nobreakpoint.png"),tr( "Remove BreakPoint" ) );
    connect( remm_brkAction, SIGNAL( triggered()),
               m_codeEditor, SLOT(slotRemBreak()), Qt::UniqueConnection );

    if( menu.exec(event->globalPos()) != 0 ) lastPos = event->pos().y();
}

#include "moc_codeeditor.cpp"
