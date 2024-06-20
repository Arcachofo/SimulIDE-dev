/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>
#include <QDebug>
#include <QStyleFactory>
#include <QProcessEnvironment>

#include "mainwindow.h"
#include "circuit.h"
#include "componentlist.h"
#include "editorwindow.h"
#include "circuitwidget.h"
#include "filewidget.h"
#include "utils.h"

MainWindow* MainWindow::m_pSelf = NULL;

MainWindow::MainWindow()
          : QMainWindow()
{
    setWindowIcon( QIcon(":/simulide.png") );
    m_pSelf   = this;
    m_circuit = NULL;
    m_autoBck = 15;
    m_state = "■";
    m_revision = 2300; /// FIXME
    m_version = "SimulIDE-"+QString( APP_VERSION )+" "+QString( REVNO );

    this->setWindowTitle( m_version );

    m_configDir.setPath( QStandardPaths::writableLocation( QStandardPaths::DataLocation ) );

    m_settings     = new QSettings( getConfigPath("simulide.ini"), QSettings::IniFormat, this );
    m_compSettings = new QSettings( getConfigPath("compList.ini"), QSettings::IniFormat, this );

    m_userDir = m_settings->value("userPath").toString();
    if( m_userDir.isEmpty() || !QDir( m_userDir ).exists() )
        m_userDir = QDir::homePath();

    // Fonts --------------------------------------
    QFontDatabase::addApplicationFont(":/Ubuntu-R.ttf" );
    QFontDatabase::addApplicationFont(":/Ubuntu-B.ttf" );
    QFontDatabase::addApplicationFont(":/UbuntuMono-B.ttf" );
    QFontDatabase::addApplicationFont(":/UbuntuMono-BI.ttf" );
    QFontDatabase::addApplicationFont(":/UbuntuMono-R.ttf" );
    QFontDatabase::addApplicationFont(":/UbuntuMono-RI.ttf" );

    float scale = 1.0;
    if( m_settings->contains( "fontScale" ) )
    {
        scale = m_settings->value( "fontScale" ).toFloat();
    }else{
        float dpiX = qApp->desktop()->logicalDpiX();
        scale = dpiX/96.0;
    }
    setFontScale( scale );

    QString fontName = "Ubuntu";
    if( m_settings->contains("fontName") ) fontName = m_settings->value("fontName").toString();
    setDefaultFontName( fontName );

    QFont df=qApp->font();
    df.setFamily( fontName );
    qApp->setFont( df );
    setFont( df );
    //----------------------------------------------

    QApplication::setStyle( QStyleFactory::create("Fusion") ); //applyStyle();
    createWidgets();
    readSettings();

    QString backPath = getConfigPath( "backup.sim1" );
    if( QFile::exists( backPath ) )
    {
        QMessageBox msgBox;
        msgBox.setText( tr("Looks like SimulIDE crashed...")+"\n\n"
                       +tr("There is an auto-saved copy of the Circuit\n")
                       +tr("You must save it with any other name if you want to keep it")+"\n\n"
                       +tr("This file will be auto-deleted!!")+"\n");
        msgBox.setInformativeText(tr("Do you want to open the auto-saved copy of the Circuit?"));
        msgBox.setStandardButtons( QMessageBox::Open | QMessageBox::Discard );
        msgBox.setDefaultButton( QMessageBox::Open );

        if( msgBox.exec() == QMessageBox::Open ) CircuitWidget::self()->loadCirc( backPath );
        else                                     QFile::remove( backPath ); // Remove backup file
    }
}
MainWindow::~MainWindow(){ }
void MainWindow::keyPressEvent( QKeyEvent* event) {
	int key = event->key();
	if( key == Qt::Key_F5 )CircuitWidget::self()->powerCircOn();
}

void MainWindow::closeEvent( QCloseEvent *event )
{
    if( !m_editor->close() )      { event->ignore(); return; }
    if( !m_circuit->newCircuit()) { event->ignore(); return; }

    writeSettings();
    event->accept();
}

void MainWindow::readSettings()
{
    restoreGeometry(                 m_settings->value("geometry" ).toByteArray());
    restoreState(                    m_settings->value("windowState" ).toByteArray());
    m_Centralsplitter->restoreState( m_settings->value("Centralsplitter/geometry").toByteArray());
    CircuitWidget::self()->splitter()->restoreState( m_settings->value("Circsplitter/geometry").toByteArray());

    m_autoBck = 15;
    if( m_settings->contains("autoBck") ) m_autoBck = m_settings->value( "autoBck" ).toInt();
    Circuit::self()->setAutoBck( m_autoBck );
}

void MainWindow::writeSettings()
{
    m_settings->setValue("autoBck",   m_autoBck );
    m_settings->setValue("fontName", m_fontName );
    m_settings->setValue("fontScale", m_fontScale );
    m_settings->setValue("geometry",  saveGeometry() );
    m_settings->setValue("windowState", saveState() );
    m_settings->setValue("Centralsplitter/geometry", m_Centralsplitter->saveState() );
    m_settings->setValue("Circsplitter/geometry", CircuitWidget::self()->splitter()->saveState() );

    ComponentList::self()->writeSettings();
    FileWidget::self()->writeSettings();
}

void MainWindow::setFontScale( float scale )
{
    if     ( scale < 0.5 ) scale = 0.5;
    else if( scale > 2 )   scale = 2;
    m_fontScale = scale;
}

QString MainWindow::loc()
{
    if( m_lang == Chinese )    return "zh_CN";
    if( m_lang == Czech )      return "cz";
    if( m_lang == Dutch )      return "nl";
    if( m_lang == French )     return "fr";
    if( m_lang == German )     return "de";
    if( m_lang == Italian )    return "it";
    if( m_lang == Russian )    return "ru";
    if( m_lang == Spanish )    return "es";
    if( m_lang == Portuguese ) return "pt_PT";
    if( m_lang == Pt_Brasil )  return "pt_BR";
    if( m_lang == Slovak )     return "sk";
    if( m_lang == Turkish )    return "tr";

    return "en";
}

void MainWindow::setLoc( QString loc )
{
    Langs lang = English;
    if     ( loc == "zh_CN" ) lang = Chinese;
    else if( loc == "cz" )    lang = Czech;
    else if( loc == "nl" )    lang = Dutch;
    else if( loc == "fr" )    lang = French;
    else if( loc == "de" )    lang = German;
    else if( loc == "it" )    lang = Italian;
    else if( loc == "ru" )    lang = Russian;
    else if( loc == "es" )    lang = Spanish;
    else if( loc == "pt_PT" ) lang = Pt_Brasil;
    else if( loc == "pt_BR" ) lang = Pt_Brasil;
    else if( loc == "sk" )    lang = Slovak;
    else if( loc == "tr" )    lang = Turkish;

    m_lang = lang;
}

void MainWindow::setLang( Langs lang ) // From appDialog
{
    m_lang = lang;

    QString langF = ":/simulide_"+loc()+".qm";
    if( !QFileInfo::exists( langF) ) m_lang = English;
    settings()->setValue( "language", loc() );
}

void MainWindow::setDefaultFontName( const QString& fontName )
{
    m_fontName = fontName;
}

void MainWindow::setFile( QString file )
{
    m_file = file;
    setWindowTitle( m_state+" "+m_version+" - "+file );
}

void MainWindow::setState( QString state )
{
    m_state = state;
    QString changed = windowTitle().endsWith("*") ? "*" : "";
    setWindowTitle( state+" "+m_version+" - "+m_file+changed );
}

void MainWindow::createWidgets()
{
    QWidget *centralWidget = new QWidget( this );
    setCentralWidget(centralWidget);

    QGridLayout *baseWidgetLayout = new QGridLayout( centralWidget );
    baseWidgetLayout->setSpacing(0);
    baseWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_Centralsplitter = new QSplitter( this );
    m_Centralsplitter->setOrientation( Qt::Horizontal );

    m_sidepanel = new QTabWidget( this );
    m_sidepanel->setTabPosition( QTabWidget::West );
    QString fontSize = QString::number( int(11*m_fontScale) );
    m_sidepanel->tabBar()->setStyleSheet("QTabBar { font-size:"+fontSize+"px; }");
    m_Centralsplitter->addWidget( m_sidepanel );

    m_componentWidget = new QWidget( this );
    m_componentWidgetLayout = new QVBoxLayout( m_componentWidget );
    m_componentWidgetLayout->setSpacing(0);
    m_componentWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* searchLayout = new QHBoxLayout( this );

    m_searchComponent = new QLineEdit( this );
    m_searchComponent->setPlaceholderText( tr("Search Components"));
    searchLayout->addWidget( m_searchComponent );
    connect( m_searchComponent, SIGNAL( editingFinished() ),
             this,              SLOT(   searchChanged()) );

    m_clearButton = new QPushButton( this );
    m_clearButton->setIcon( QIcon(":/remove.svg") );
    m_clearButton->setToolTip( tr("Clear search"));
    searchLayout->addWidget( m_clearButton );
    connect( m_clearButton, SIGNAL( clicked() ),
             this,          SLOT(   clearSearch()) );

    m_componentWidgetLayout->addLayout( searchLayout );

    m_fileSystemTree = new FileWidget( this );
    m_circuit = new CircuitWidget( this );

    m_components = new ComponentList( m_sidepanel );
    m_componentWidgetLayout->addWidget( m_components );

    m_sidepanel->addTab( m_componentWidget, tr("Components") );
    m_sidepanel->addTab( m_fileSystemTree, tr( "File explorer" ) );

    m_Centralsplitter->addWidget( m_circuit );

    m_editor = new EditorWindow( this );
    m_editor->setObjectName(QString::fromUtf8("editor"));
    m_Centralsplitter->addWidget( m_editor );

    baseWidgetLayout->addWidget( m_Centralsplitter, 0, 0 );

    m_Centralsplitter->setSizes( {150, 350, 500} );

    this->showMaximized();
}

void MainWindow::clearSearch()
{
    m_searchComponent->clear();
    searchChanged();
}

void MainWindow::searchChanged()
{
    QString filter = m_searchComponent->text();
    m_components->search( filter );
}

QString MainWindow::getHelp( QString name, bool save )
{
    if( save && m_help.contains( name ) ) return m_help.value( name );

    QString help = tr("No help available");
    QString locale = loc();
    QString localeFolder = "";

    if( loc() != "en" ) {
        locale.prepend("_");
        localeFolder = locale + "/";
    }
    else locale = "";

    name = name.toLower().replace( " ", "" );
    QString dfPath = getDataFilePath("help/"+localeFolder+name+locale+".txt");

    if( !QFileInfo::exists( dfPath ) ) dfPath = getDataFilePath( "help/"+name+".txt" );
    if( QFileInfo::exists( dfPath ) )
    {
        help.clear();
        QStringList lines = fileToStringList( dfPath, "MainWindow::getHelp" );
        for( QString line : lines )
        {
            if( line.startsWith("#include") )
            {
                QString file = line.remove("#include ");
                line = getHelp( file );
                help.append( line );
            }
            else help.append( line+"\n" );
        }
    }
    if( save ) m_help[name] = help;
    return help;
}

void MainWindow::getUserPath()
{
    QString path = getDirDialog( tr("Select User data directory"), m_userDir );

    setUserPath( path );
}

void MainWindow::setUserPath( QString path )
{
    if( !QFileInfo::exists( path ) ) return;
    m_settings->setValue("userPath", path);
    m_userDir = path;
}

QString MainWindow::getUserFilePath( QString f )
{
    if( m_userDir.isEmpty() ) return "";
    return QDir( m_userDir ).absoluteFilePath( f );
}

//QString MainWindow::getFilePath( QString file )   { return m_filesDir.absoluteFilePath( file ); }
QString MainWindow::getConfigPath( QString file ) { return m_configDir.absoluteFilePath( file ); }
QString MainWindow::getDataFilePath( QString file )
{
    QString path;
    if( Circuit::self() )
    {
        QString circPath = Circuit::self()->getFilePath();
        if( !circPath.isEmpty() )
        {
            QDir circuitDir = QFileInfo( circPath ).absoluteDir();
            path = circuitDir.absoluteFilePath("data/"+file );
            if( QFileInfo::exists( path ) ) return path;                     // File in Circuit data folder
            path = "";
        }
    }
    if( path.isEmpty() ) path = MainWindow::self()->getUserFilePath( file ); // File in user data folder
    //if( path.isEmpty() || !QFileInfo::exists( path ) )
    //    path = getFilePath("data/"+file );                    // File in SimulIDE data folder

    return path;
}
QSettings* MainWindow::settings() { return m_settings; }
QSettings* MainWindow::compSettings() { return m_compSettings; }

#include  "moc_mainwindow.cpp"
