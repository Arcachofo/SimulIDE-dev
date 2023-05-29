/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QWidget>
#include <QTabWidget>
#include <QTabBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QGridLayout>
#include <QSplitter>
#include <QToolButton>
#include <QSettings>

#include "editorwidget.h"
#include "findreplace.h"
#include "scrollbar.h"
#include "editorprop.h"
#include "basedebugger.h"
#include "mainwindow.h"
#include "simulator.h"
#include "utils.h"

EditorWidget::EditorWidget( QWidget* parent )
            : QWidget( parent )
            , m_outPane( this )
            , m_settingsMenu( this )
            , m_fileMenu( this )
{
    m_editDialog = NULL;

    setAcceptDrops( true );

    createActions();
    createToolBars();
    createWidgets();

    updateRecentFileActions();
    readSettings();
}
EditorWidget::~EditorWidget(){}

CodeEditor* EditorWidget::getCodeEditor()
{
    return (CodeEditor*)m_docWidget->currentWidget();
}

/*CodeEditor* EditorWidget::getCodeEditor( QString file )
{
    return (CodeEditor*)m_fileList.value( file );
}*/

bool EditorWidget::close()
{
    writeSettings();

    for( int i=0; i<m_docWidget->count(); i++ )
        closeTab( m_docWidget->currentIndex() );

    return maybeSave();
}

void EditorWidget::newFile()
{
    CodeEditor* codeEditor = new CodeEditor( this, &m_outPane );

    codeEditor->setVerticalScrollBar( new scrollWidget( codeEditor, Qt::Vertical ) );

    m_docWidget->addTab( codeEditor, "New" );
    m_docWidget->setCurrentWidget( codeEditor );

    connect( codeEditor->document(), SIGNAL( contentsChanged()),
             this,                   SLOT(   documentWasModified()), Qt::UniqueConnection);

    m_fileList["New"] = codeEditor;
    codeEditor->document()->setModified( true );
    documentWasModified();
    enableFileActs( true );
    enableDebugActs( true );
}

void EditorWidget::open()
{
    const QString dir = m_lastDir;
    QString fileName = QFileDialog::getOpenFileName( this, tr("Load File"), dir,
                       tr("All files")+" (*);;Arduino (*.ino);;Asm (*.asm);;GcBasic (*.gcb)" );

    if( !fileName.isEmpty() ) loadFile( fileName );
}

void EditorWidget::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>( sender() );
    if( action )
    {
        QString file = action->data().toString();
        QFile pfile( file );
        if( pfile.exists() ) loadFile( file );
        else{
            const QMessageBox::StandardButton ret
            = QMessageBox::warning( this, "EditorWidget::openRecentFile",
                                   tr("\nCan't find file:\n")+
                                   file+"\n\n"+
                                   tr("Do you want to remove it from Recent Files?\n"),
              QMessageBox::Yes | QMessageBox::No );

            if( ret == QMessageBox::Yes )
            {
                QSettings* settings = MainWindow::self()->settings();
                QStringList files = settings->value("recentFileList").toStringList();
                files.removeAll( file );
                settings->setValue("recentFileList", files );
                updateRecentFileActions();
            }
        }
    }
}

void EditorWidget::dragEnterEvent( QDragEnterEvent* event)
{
    event->accept();
}

void EditorWidget::dropEvent( QDropEvent* event )
{
    QString id = event->mimeData()->text();

    QString file = "file://";
    if( id.startsWith( file ) )
    {
        id.replace( file, "" ).replace("\r\n", "" ).replace("%20", " ");
#ifdef _WIN32
        if( id.startsWith( "/" )) id.remove( 0, 1 );
#endif
        loadFile( id );
    }
}

void EditorWidget::keyPressEvent( QKeyEvent* event )
{
    if( event->modifiers() & Qt::ControlModifier )
    {
        if( event->key() == Qt::Key_N )
        {
            newFile();
        }
        else if( event->key() == Qt::Key_S )
        {
            if( event->modifiers() & Qt::ShiftModifier) saveAs();
            else                                        save();
        }
        else if( event->key() == Qt::Key_O )
        {
            open();
        }
        else if( event->key() == Qt::Key_R )
        {
            reload();
        }
    }
}

void EditorWidget::loadFile( const QString &fileName )
{
    if( !QFileInfo::exists( fileName ) )
    {
        m_outPane.appendLine( tr("File doesn't exist")+":\n"+fileName );
        return;
    }
    if( m_fileList.contains( fileName ) )
    {
        m_docWidget->setCurrentWidget( m_fileList.value( fileName ) );
        return;
    }
    newFile();
    QApplication::setOverrideCursor( Qt::WaitCursor );

    CodeEditor* ce = getCodeEditor();
    ce->setPlainText( fileToString( fileName, "EditorWidget::loadFile" ) );
    ce->setFile( fileName );

    QFile file( fileName +".brk" );
    if( file.exists() ) loadBreakpoints( fileName +".brk" );

    m_lastDir = fileName;

    int index = m_docWidget->currentIndex();
    m_fileList.remove("New");
    m_fileList[fileName] = ce;
    m_docWidget->setTabText( index, getFileName(fileName) );

    enableFileActs( true );
    enableDebugActs( true );

    QSettings* settings = MainWindow::self()->settings();
    QStringList files = settings->value("recentFileList").toStringList();
    files.removeAll( fileName );
    files.prepend( fileName );
    while( files.size() > MaxRecentFiles ) files.removeLast();
    settings->setValue("recentFileList", files );
    updateRecentFileActions();

    QApplication::restoreOverrideCursor();
}

void EditorWidget::reload()
{
    QString fileName = m_fileList.key( m_docWidget->currentWidget() );
    loadFile( fileName );
}

bool EditorWidget::save()
{
    QString file = getCodeEditor()->getFilePath();
    if( file.isEmpty() ) return saveAs();
    else                 return saveFile( file );
}

bool EditorWidget::saveAs()
{
    CodeEditor* ce = getCodeEditor();

    QFileInfo fi = QFileInfo( ce->getFilePath() );
    QString ext  = fi.suffix();
    QString path = fi.absolutePath();
    if( path == "" ) path = m_lastDir;

    QString extensions = "";
    if( ext == "" ) extensions = tr("All files")+" (*);;Arduino (*.ino);;Asm (*.asm);;GcBasic (*.gcb)";
    else            extensions = "."+ext+"(*."+ext+");;"+tr("All files")+" (*.*)";

    QString fileName = QFileDialog::getSaveFileName( this, tr("Save Document As"), path, extensions );
    if( fileName.isEmpty() ) return false;

    m_fileList[fileName] = ce;

    return saveFile( fileName );
}

bool EditorWidget::saveFile( const QString &fileName )
{
    QFile file( fileName );
    if( !file.open( QFile::WriteOnly | QFile::Text) )
    {
        QMessageBox::warning(this, "EditorWindow::saveFile",
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QTextStream out( &file );
    out.setCodec("UTF-8");

    CodeEditor* ce = getCodeEditor();
    out << ce->toPlainText();
    file.close();

    saveBreakpoints( fileName );

    ce->document()->setModified( false );
    documentWasModified();

    m_docWidget->setTabText( m_docWidget->currentIndex(), getFileName(fileName) );
    ce->setFile( fileName );

    QApplication::restoreOverrideCursor();
    return true;
}

void EditorWidget::saveBreakpoints( QString fileName )
{
    fileName = fileName +".brk";
    QFile file( fileName );
    if( file.exists() ) file.remove();

    QList<int>* brkList = getCodeEditor()->getBreakPoints();
    if( brkList->isEmpty() ) return;

    QString brkListStr;
    for( int brk : *brkList )
        brkListStr.append( QString::number( brk )+"," );

    if( !file.open( QFile::WriteOnly | QFile::Text) )
    {
        QMessageBox::warning(this, "EditorWindow::saveBreakpoints",
                             tr("Cannot write file %1:\n%2.")
                             .arg( fileName )
                             .arg( file.errorString() ));
        return;
    }
    QTextStream out( &file );
    out.setCodec("UTF-8");
    out << brkListStr;
    file.close();
}

void EditorWidget::loadBreakpoints( const QString &fileName )
{
    QString brkListStr = fileToString( fileName, "EditorWidget::loadBreakpoints" );
    QStringList list = brkListStr.split(",");
    list.removeOne("");

    CodeEditor* ce = getCodeEditor();
    for( QString brk : list ) ce->addBreakPoint( brk.toInt() );
}

bool EditorWidget::maybeSave()
{
    if( m_fileList.isEmpty() ) return true;
    if( getCodeEditor()->document()->isModified() )
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning( this, "EditorWindow::saveFile",
              tr("\nThe Document has been modified.\nDo you want to save your changes?\n"),
              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if     ( ret == QMessageBox::Save )   return save();
        else if( ret == QMessageBox::Cancel ) return false;
    }
    return true;
}

void EditorWidget::documentWasModified()
{
    CodeEditor* ce = getCodeEditor();
    QTextDocument *doc = ce->document();

    bool    modified = doc->isModified();
    int     index    = m_docWidget->currentIndex();
    QString tabText  = m_docWidget->tabText( index );

    if     ( modified && !tabText.endsWith("*") ) tabText.append("*");
    else if( !modified && tabText.endsWith("*") ) tabText.remove("*");

    m_docWidget->setTabText( index, tabText );

    redoAct->setEnabled( false );
    undoAct->setEnabled( false );
    if( doc->isRedoAvailable() ) redoAct->setEnabled( true );
    if( doc->isUndoAvailable() ) undoAct->setEnabled( true );
}

/*void EditorWidget::tabChanged( int tab )
{
    //qDebug() << "EditorWindow::tabChanged" << m_docWidget->currentIndex() << tab;
}*/

void EditorWidget::closeTab( int index )
{
    if( m_debuggerToolBar->isVisible() ) stop();

    m_docWidget->setCurrentIndex( index );
    if( !maybeSave() ) return;

    CodeEditor* ce = getCodeEditor();
    m_fileList.remove( m_fileList.key( ce ) );
    m_docWidget->removeTab( index );
    delete ce;

    if( m_fileList.isEmpty() )  // disable file actions
    {
        enableFileActs( false );
        enableDebugActs( false );
    }

    int last = m_docWidget->count()-1;
    if( index > last ) m_docWidget->setCurrentIndex( last );
    else               m_docWidget->setCurrentIndex( index );
}

void EditorWidget::confEditor()
{
    if( !m_editDialog ) m_editDialog = new EditorProp( this );
    m_editDialog->show();
}

void EditorWidget::confCompiler()
{
    CodeEditor* ce = getCodeEditor();
    if( !ce ) return;
    BaseDebugger* comp = ce->getCompiler();
    if( comp ) comp->compProps();
}

void EditorWidget::updateRecentFileActions()
{
    QSettings* settings = MainWindow::self()->settings();
    QStringList files = settings->value("recentFileList").toStringList();

    int numRecentFiles = qMin( files.size(), (int)MaxRecentFiles );

    for( int i=0; i<numRecentFiles; i++ )
    {
        QString text = tr("&%1 %2").arg(i + 1).arg( getFileName( files[i] ) );
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData( files[i] );
        recentFileActs[i]->setVisible( true );
    }
    for( int i=numRecentFiles; i<MaxRecentFiles; i++ ) recentFileActs[i]->setVisible(false);
}

void EditorWidget::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();

    restoreGeometry( settings->value("geometry").toByteArray() );
    m_docWidget->restoreGeometry( settings->value("docWidget/geometry").toByteArray() );
    m_lastDir = settings->value("lastDir").toString();
}

void EditorWidget::writeSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    settings->setValue( "geometry", saveGeometry() );
    settings->setValue( "docWidget/geometry", m_docWidget->saveGeometry() );
    settings->setValue( "lastDir", m_lastDir );
}

void EditorWidget::findReplaceDialog()
{
    CodeEditor* ce = getCodeEditor();

    m_findRepDialog->setEditor( ce );

    QString text =ce->textCursor().selectedText();
    if( text != "" ) m_findRepDialog->setTextToFind( text );

    m_findRepDialog->show();
}

void EditorWidget::enableFileActs( bool enable )
{
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    cutAct->setEnabled( enable );
    copyAct->setEnabled( enable );
    pasteAct->setEnabled( enable );
    undoAct->setEnabled( enable );
    redoAct->setEnabled( enable );
    findQtAct->setEnabled( enable );
}

void EditorWidget::enableDebugActs( bool enable )
{
    debugAct->setEnabled( enable );
    runAct->setEnabled( enable );
    stepAct->setEnabled( enable );
    stepOverAct->setEnabled( enable );
    pauseAct->setEnabled( enable );
    resetAct->setEnabled( enable );
    stopAct->setEnabled( enable );
    compileAct->setEnabled( enable );
    loadAct->setEnabled( enable );
}

void EditorWidget::setStepActs(  bool s )
{
    runAct->setEnabled( s );
    stepAct->setEnabled( s );
    stepOverAct->setEnabled( s );
    resetAct->setEnabled( s );
    pauseAct->setEnabled( !s );
}

void EditorWidget::createWidgets()
{
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* vLayout = new QVBoxLayout( this );
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);

    hLayout->addWidget( m_editorToolBar );
    hLayout->addWidget( m_findToolBar );
    hLayout->addWidget( m_compileToolBar );
    hLayout->addWidget( m_debuggerToolBar );
    hLayout->addStretch();
    vLayout->addLayout( hLayout );

    QSplitter* splitter0 = new QSplitter( this );
    splitter0->setObjectName("splitter0");
    splitter0->setOrientation( Qt::Vertical );
    vLayout->addWidget( splitter0 );

    m_docWidget = new QTabWidget( this );
    m_docWidget->setObjectName("docWidget");
    m_docWidget->setTabPosition( QTabWidget::North );
    m_docWidget->setTabsClosable ( true );
    m_docWidget->setContextMenuPolicy( Qt::CustomContextMenu );

    double fontScale = MainWindow::self()->fontScale();
    QString fontSize = QString::number( int(10*fontScale) );
    m_docWidget->tabBar()->setStyleSheet("QTabBar { font-size:"+fontSize+"px; }");
    m_docWidget->setMovable( true );

    splitter0->addWidget( m_docWidget );
    splitter0->addWidget( &m_outPane );
    splitter0->setSizes( {300, 100} );

    connect( m_docWidget, SIGNAL( tabCloseRequested(int)),
             this,        SLOT(   closeTab(int)), Qt::UniqueConnection);

    //connect( m_docWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)), Qt::UniqueConnection);

    m_findRepDialog = new FindReplace( this );
    m_findRepDialog->setModal( false );
}

void EditorWidget::createActions()
{
    confEditAct = new QAction(QIcon(":/blank.png"), tr("Editor Settings"), this);
    confEditAct->setStatusTip(tr("Editor Settings"));
    connect( confEditAct, SIGNAL(triggered()), this, SLOT(confEditor()), Qt::UniqueConnection);

    confCompAct = new QAction(QIcon(":/blank.png"), tr("Compiler Settings"), this);
    confCompAct->setStatusTip(tr("Compiler Settings"));
    connect( confCompAct, SIGNAL(triggered()), this, SLOT(confCompiler()), Qt::UniqueConnection);

    for( int i=0; i<MaxRecentFiles; i++ )
    {
        recentFileActs[i] = new QAction( this );
        recentFileActs[i]->setVisible( false );
        connect( recentFileActs[i], SIGNAL( triggered() ),
                 this,              SLOT( openRecentFile() ), Qt::UniqueConnection);
    }

    newAct = new QAction(QIcon(":/new.svg"), tr("&New\tCtrl+N"), this);
    newAct->setStatusTip(tr("Create a new file"));
    connect( newAct, SIGNAL(triggered()), this, SLOT(newFile()), Qt::UniqueConnection);

    openAct = new QAction(QIcon(":/open.svg"), tr("&Open...\tCtrl+O"), this);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()), Qt::UniqueConnection);

    saveAct = new QAction(QIcon(":/save.svg"), tr("&Save\tCtrl+S"), this);
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()), Qt::UniqueConnection);

    saveAsAct = new QAction(QIcon(":/saveas.svg"),tr("Save &As...\tCtrl+Shift+S"), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()), Qt::UniqueConnection);

    cutAct = new QAction(QIcon(":/cut.svg"), tr("Cu&t\tCtrl+X"), this);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    cutAct->setEnabled(false);
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()), Qt::UniqueConnection);

    copyAct = new QAction(QIcon(":/copy.svg"), tr("&Copy\tCtrl+C"), this);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    copyAct->setEnabled(false);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()), Qt::UniqueConnection);

    pasteAct = new QAction(QIcon(":/paste.svg"), tr("&Paste\tCtrl+V"), this);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    pasteAct->setEnabled(false);
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()), Qt::UniqueConnection);

    undoAct = new QAction(QIcon(":/undo.svg"), tr("Undo\tCtrl+Z"), this);
    undoAct->setStatusTip(tr("Undo the last action"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()), Qt::UniqueConnection);

    redoAct = new QAction(QIcon(":/redo.svg"), tr("Redo\tCtrl+Shift+Z"), this);
    redoAct->setStatusTip(tr("Redo the last action"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    runAct =  new QAction(QIcon(":/runtobk.png"),tr("Run To Breakpoint"), this);
    runAct->setStatusTip(tr("Run to next breakpoint"));
    runAct->setEnabled(false);
    connect(runAct, SIGNAL(triggered()), this, SLOT(run()), Qt::UniqueConnection);

    stepAct = new QAction(QIcon(":/step.svg"),tr("Step"), this);
    stepAct->setStatusTip(tr("Step debugger"));
    stepAct->setEnabled(false);
    connect( stepAct, SIGNAL(triggered()), this, SLOT(step()), Qt::UniqueConnection );

    stepOverAct = new QAction(QIcon(":/rotatecw.svg"),tr("StepOver"), this);
    stepOverAct->setStatusTip(tr("Step Over"));
    stepOverAct->setEnabled(false);
    stepOverAct->setVisible(false);
    connect( stepOverAct, SIGNAL(triggered()), this, SLOT(stepOver()), Qt::UniqueConnection );

    pauseAct = new QAction(QIcon(":/pause.svg"),tr("Pause"), this);
    pauseAct->setStatusTip(tr("Pause debugger"));
    pauseAct->setEnabled(false);
    connect( pauseAct, SIGNAL(triggered()), this, SLOT(pause()), Qt::UniqueConnection );

    resetAct = new QAction(QIcon(":/reset.svg"),tr("Reset"), this);
    resetAct->setStatusTip(tr("Reset debugger"));
    resetAct->setEnabled(false);
    connect( resetAct, SIGNAL(triggered()), this, SLOT(reset()), Qt::UniqueConnection );

    stopAct = new QAction(QIcon(":/stop.svg"),tr("Stop Debugger"), this);
    stopAct->setStatusTip(tr("Stop debugger"));
    stopAct->setEnabled(false);
    connect( stopAct, SIGNAL(triggered()), this, SLOT(stop()), Qt::UniqueConnection );

    compileAct = new QAction(QIcon(":/verify.svg"),tr("Compile"), this);
    compileAct->setStatusTip(tr("Compile Source"));
    compileAct->setEnabled(false);
    connect( compileAct, SIGNAL(triggered()), this, SLOT(compile()), Qt::UniqueConnection );

    loadAct = new QAction(QIcon(":/upload.svg"),tr("UpLoad"), this);
    loadAct->setStatusTip(tr("Load Firmware"));
    loadAct->setEnabled(false);
    connect( loadAct, SIGNAL(triggered()), this, SLOT(upload()), Qt::UniqueConnection );

    findQtAct = new QAction(QIcon(":/find.svg"),tr("Find Replace"), this);
    findQtAct->setStatusTip(tr("Find Replace"));
    findQtAct->setEnabled(false);
    connect(findQtAct, SIGNAL(triggered()), this, SLOT(findReplaceDialog()), Qt::UniqueConnection);

    debugAct =  new QAction(QIcon(":/debug.svg"),tr("Debug"), this);
    debugAct->setStatusTip(tr("Start Debugger"));
    debugAct->setEnabled(false);
    connect(debugAct, SIGNAL(triggered()), this, SLOT(debug()), Qt::UniqueConnection);
}

void EditorWidget::createToolBars()
{
    m_settingsMenu.addAction( confEditAct );
    m_settingsMenu.addAction( confCompAct );

    QToolButton* settingsButton = new QToolButton( this );
    settingsButton->setToolTip( tr("Settings") );
    settingsButton->setMenu( &m_settingsMenu );
    settingsButton->setIcon( QIcon(":/config.svg") );
    settingsButton->setPopupMode( QToolButton::InstantPopup );

    for( int i=0; i<MaxRecentFiles; i++ ) m_fileMenu.addAction( recentFileActs[i] );
    QToolButton* fileButton = new QToolButton( this );
    fileButton->setToolTip( tr("Last Files") );
    fileButton->setMenu( &m_fileMenu );
    fileButton->setIcon( QIcon(":/lastfiles.svg") );
    fileButton->setPopupMode( QToolButton::InstantPopup );

    double fs = MainWindow::self()->fontScale()*20;

    m_editorToolBar = new QToolBar( this );
    m_editorToolBar->setIconSize( QSize( fs, fs ) );
    m_editorToolBar->addWidget( settingsButton );
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth( 15 );
    m_editorToolBar->addWidget( spacer );
    m_editorToolBar->addSeparator();//..........................
    m_editorToolBar->addWidget( fileButton );

    m_editorToolBar->addAction(newAct);
    m_editorToolBar->addAction(openAct);
    m_editorToolBar->addAction(saveAct);
    m_editorToolBar->addAction(saveAsAct);
    m_editorToolBar->addSeparator();

    m_findToolBar = new QToolBar( this );
    m_findToolBar->setIconSize( QSize( fs, fs ) );
    m_findToolBar->addAction(findQtAct);
    m_findToolBar->addSeparator();

    m_compileToolBar = new QToolBar( this );
    m_compileToolBar->setIconSize( QSize( fs, fs ) );
    m_compileToolBar->addAction(compileAct);
    m_compileToolBar->addAction(loadAct);
    m_compileToolBar->addSeparator();
    m_compileToolBar->addAction(debugAct);

    m_debuggerToolBar = new QToolBar( this );
    m_debuggerToolBar->setIconSize( QSize( fs, fs ) );
    m_debuggerToolBar->addAction(stepAct);
    m_debuggerToolBar->addAction(stepOverAct);
    m_debuggerToolBar->addAction(runAct);
    m_debuggerToolBar->addAction(pauseAct);
    m_debuggerToolBar->addAction(resetAct);
    m_debuggerToolBar->addSeparator();
    m_debuggerToolBar->addAction(stopAct);
    m_debuggerToolBar->setVisible( false );
}

#include  "moc_editorwidget.cpp"
