/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QFileSystemModel>

#include "filebrowser.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "circuit.h"
#include "filewidget.h"
#include "editorwindow.h"
#include "componentlist.h"

FileBrowser* FileBrowser::m_pSelf = NULL;

FileBrowser::FileBrowser( QWidget *parent ) 
           : QTreeView( parent )
{
    m_pSelf = this;
    m_showHidden = false;
    
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setNameFilterDisables( false );
    m_fileSystemModel->setRootPath( QDir::rootPath() );
    
    m_currentPath = QDir::rootPath();
    
    setModel( m_fileSystemModel );
    setRootIndex( m_fileSystemModel->index( QDir::rootPath() ));
    
    setHeaderHidden( true );
    hideColumn( 1 );
    hideColumn( 2 );
    hideColumn( 3 );
    
    double fontScale = MainWindow::self()->fontScale();
    QString fontSize = QString::number( int(11*fontScale) );

    setObjectName( "FileBrowser" );
    setStyleSheet("font-size:"+fontSize+"px; border: 0px solid red");
}

FileBrowser::~FileBrowser() { }

void FileBrowser::cdUp()
{
    QModelIndex currentDir = m_fileSystemModel->index( m_currentPath );
    setPath( m_fileSystemModel->filePath( currentDir.parent() ) );
}

void FileBrowser::openInEditor()
{
    QString path = m_fileSystemModel->filePath( currentIndex() );
    EditorWindow::self()->loadFile( path );
}

void FileBrowser::open()
{
    QString path = m_fileSystemModel->filePath( currentIndex() );
    if( path.isEmpty() ) return;
    
    if( m_fileSystemModel->isDir( currentIndex() ) ) setPath( path );
    else{
        if( path.endsWith(".simu")
         || path.endsWith(".sim1")
         || path.endsWith(".comp") ) CircuitWidget::self()->loadCirc( path );
        else                         EditorWindow::self()->loadFile( path );
    }
}

void FileBrowser::setPath( QString path )
{
    m_currentPath = path;

    FileWidget::self()->setPath( path );
    QModelIndex index = m_fileSystemModel->index( path );
    
    if( path == QDir::rootPath() ) index = index.parent();
    setRootIndex( index );

    if( m_showHidden ) m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden );
    else               m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs );
    m_fileSystemModel->setNameFilters({"*"});
    this->collapseAll();
}

void FileBrowser::addBookMark()
{
    QString filePath = m_fileSystemModel->filePath( currentIndex() );
    FileWidget::self()->addBookMark( filePath );
}

void FileBrowser::showHidden()
{
    m_showHidden = !m_showHidden;
    if( m_showHidden ) m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden );
    else               m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs );
}

void FileBrowser::searchFiles( QString filter )
{
    if( filter.isEmpty() )
    {
        if( m_showHidden ) m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden );
        else               m_fileSystemModel->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs );
        m_fileSystemModel->setNameFilters({"*"});
        this->collapseAll();
    }else{
        this->expandAll();
        m_fileSystemModel->setNameFilters({filter});
        m_fileSystemModel->setFilter( QDir::NoDotAndDotDot | QDir::Files );
    }
}

void FileBrowser::mouseDoubleClickEvent( QMouseEvent* event )
{
    event->accept();
    open();
}

void FileBrowser::convert()
{
    QString path = m_fileSystemModel->filePath( currentIndex() );
    ComponentList::self()->loadXml( path, true );
}

void FileBrowser::contextMenuEvent( QContextMenuEvent* event )
{
    QTreeView::contextMenuEvent( event );

    if( !event->isAccepted() )
    {
        event->accept();
        QPoint eventPos = event->globalPos();
        QMenu menu;
        
        if( m_fileSystemModel->isDir( currentIndex()) )
        {
            QAction* addBookMarkAction = menu.addAction(QIcon(":/setroot.png"),tr("Add Bookmark"));
            connect( addBookMarkAction, SIGNAL( triggered()), 
                     this,              SLOT(   addBookMark() ), Qt::UniqueConnection );
                     
            menu.addSeparator();
        }else{
            QAction* openWithEditor = menu.addAction(QIcon(":/open.png"),tr("Open in editor"));
            connect( openWithEditor, SIGNAL( triggered()), 
                     this,           SLOT(   openInEditor()), Qt::UniqueConnection );
                     
            menu.addSeparator();

            QString path = m_fileSystemModel->filePath( currentIndex() );
            if( path.endsWith(".xml") )
            {
                QAction* convertXml = menu.addAction(QIcon(":/open.png"),tr("Convert components"));
                connect( convertXml, SIGNAL( triggered()),
                         this,       SLOT(   convert()), Qt::UniqueConnection );
            }
        }
        QAction* showHidden = menu.addAction( tr("Show Hidden"));
        showHidden->setCheckable( true );
        showHidden->setChecked( m_showHidden );
        connect( showHidden, SIGNAL( triggered()), 
                 this,       SLOT(   showHidden()), Qt::UniqueConnection );
        menu.exec( eventPos );
}   }

void FileBrowser::keyPressEvent( QKeyEvent *event )
{
    bool isEnter = ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return));
    if( isEnter ) open();
}

#include  "moc_filebrowser.cpp"
