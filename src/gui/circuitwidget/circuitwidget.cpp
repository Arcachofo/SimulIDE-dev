/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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
 
#include "circuitwidget.h"
#include "mainwindow.h"
#include "simulator.h"
#include "circuit.h"
#include "appprop.h"
#include "circprop.h"
#include "simuprop.h"
#include "filebrowser.h"
#include "about.h"
#include "utils.h"

CircuitWidget*  CircuitWidget::m_pSelf = 0l;

CircuitWidget::CircuitWidget( QWidget *parent  )
             : QWidget( parent )
             , m_verticalLayout( this )
             , m_circView( this )
             , m_outPane( this )
             , m_circToolBar( this )
             , m_fileMenu( this )
             , m_settingsMenu( this )
             , m_infoMenu( this )
{
    setObjectName( "CircuitWidget" );
    m_pSelf = this;

    m_cirPropW = NULL;
    m_simPropW = NULL;
    m_appPropW = NULL;
    m_about = NULL;

    m_verticalLayout.setObjectName( "verticalLayout" );
    m_verticalLayout.setContentsMargins(0, 0, 0, 0);
    m_verticalLayout.setSpacing(0);

    m_verticalLayout.addWidget( &m_circToolBar );

    m_splitter = new QSplitter( this );
    m_splitter->setObjectName("CircuitSplitter");
    m_splitter->setOrientation( Qt::Vertical );
    m_verticalLayout.addWidget( m_splitter );

    m_splitter->addWidget( &m_circView );
    m_splitter->addWidget( &m_outPane );
    m_splitter->setSizes( {500, 100} );

    m_rateLabel = new QLabel( this );
    QFont font( "Arial", 10, QFont::Normal );
    double fontScale = MainWindow::self()->fontScale();
    font.setPixelSize( int(10*fontScale) );
    m_rateLabel->setFont( font );

    m_msgLabel  = new QLabel( this );
    m_msgLabel->setFont( font );
    m_msgLabel->setMaximumSize( 200, 15 );

    createActions();
    updateRecentFileActions();
    createToolBars();
    
    QString appPath = QCoreApplication::applicationDirPath();
    
    m_lastCircDir = MainWindow::self()->settings()->value("lastCircDir").toByteArray();
    if( m_lastCircDir.isEmpty() )  m_lastCircDir = appPath + "..share/simulide/examples";
    
    newCircuit();
    setRate( 0, 0 );
}
CircuitWidget::~CircuitWidget() { }

void CircuitWidget::clear()
{
    if( m_cirPropW )
    {
        m_cirPropW->setParent( NULL );
        m_cirPropW->close();
        m_cirPropW->deleteLater();
        m_cirPropW = NULL;
    }
    if( m_simPropW )
    {
        m_simPropW->setParent( NULL );
        m_simPropW->close();
        m_simPropW->deleteLater();
        m_simPropW = NULL;
    }
    m_circView.clear();
    m_circView.setCircTime( 0 );
}

void CircuitWidget::createActions()
{
    for( int i=0; i<MaxRecentFiles; i++ )
    {
        recentFileActs[i] = new QAction( this );
        recentFileActs[i]->setVisible( false );
        connect( recentFileActs[i], SIGNAL( triggered() ),
                 this,              SLOT( openRecentFile() ), Qt::UniqueConnection );
    }

    newCircAct = new QAction( QIcon(":/newcirc.png"), tr("New C&ircuit\tCtrl+N"), this);
    newCircAct->setStatusTip( tr("Create a new Circuit"));
    connect( newCircAct, SIGNAL( triggered()),
                   this, SLOT( newCircuit()), Qt::UniqueConnection );

    openCircAct = new QAction( QIcon(":/opencirc.png"), tr("&Open Circuit\tCtrl+O"), this);
    openCircAct->setStatusTip( tr("Open an existing Circuit"));
    connect( openCircAct, SIGNAL( triggered()),
                    this, SLOT(openCirc()), Qt::UniqueConnection );

    saveCircAct = new QAction( QIcon(":/savecirc.png"), tr("&Save Circuit\tCtrl+S"), this);
    saveCircAct->setStatusTip( tr("Save the Circuit to disk"));
    connect( saveCircAct, SIGNAL( triggered()),
                    this, SLOT(saveCirc()), Qt::UniqueConnection );

    saveCircAsAct = new QAction( QIcon(":/savecircas.png"),tr("Save Circuit &As...\tCtrl+Shift+S"), this);
    saveCircAsAct->setStatusTip( tr("Save the Circuit under a new name"));
    connect( saveCircAsAct, SIGNAL( triggered()),
                      this, SLOT(saveCircAs()), Qt::UniqueConnection );

    powerCircAct = new QAction( QIcon(":/poweroff.png"),tr("Power Circuit"), this);
    powerCircAct->setStatusTip(tr("Power the Circuit"));
    powerCircAct->setIconText("Off");
    connect( powerCircAct, SIGNAL( triggered()),
                     this, SLOT(powerCirc()), Qt::UniqueConnection );

    pauseSimAct = new QAction( QIcon(":/pausesim.png"),tr("Pause Simulation"), this);
    pauseSimAct->setStatusTip(tr("Pause Simulation"));
    connect( pauseSimAct, SIGNAL( triggered()),
             this, SLOT(pauseSim()), Qt::UniqueConnection );

    settAppAct = new QAction( QIcon(""),tr("App Settings"), this);
    settAppAct->setStatusTip(tr("App Settings"));
    connect( settAppAct, SIGNAL( triggered()),
                   this, SLOT(settApp()), Qt::UniqueConnection );

    settCirAct = new QAction( QIcon(""),tr("Circuit Settings"), this);
    settCirAct->setStatusTip(tr("Circuit Settings"));
    connect( settCirAct, SIGNAL( triggered()),
                   this, SLOT(settCir()), Qt::UniqueConnection );

    settSimAct = new QAction( QIcon(""),tr("Simulation Settings"), this);
    settSimAct->setStatusTip(tr("Simulation Settings"));
    connect( settSimAct, SIGNAL(triggered()),
                   this, SLOT(settSim()), Qt::UniqueConnection );
    
    infoAct = new QAction( QIcon(":/help.png"),tr("Online Help"), this);
    infoAct->setStatusTip(tr("Online Help"));
    connect( infoAct, SIGNAL( triggered()),
                this, SLOT(openInfo()), Qt::UniqueConnection );
    
    aboutAct = new QAction( QIcon(":/about.png"),tr("About SimulIDE"), this);
    aboutAct->setStatusTip(tr("About SimulIDE"));
    connect( aboutAct, SIGNAL( triggered()),
                 this, SLOT(about()), Qt::UniqueConnection );
    
    aboutQtAct = new QAction( QIcon(":/about.png"),tr("About Qt"), this);
    aboutQtAct->setStatusTip(tr("About Qt"));
    connect( aboutQtAct, SIGNAL(triggered()),
                   qApp, SLOT(aboutQt()), Qt::UniqueConnection );
}

void CircuitWidget::createToolBars()
{
    m_settingsMenu.addAction( settAppAct );
    m_settingsMenu.addAction( settCirAct );
    m_settingsMenu.addAction( settSimAct );

    QToolButton* settingsButton = new QToolButton( this );
    settingsButton->setToolTip( tr("Settings") );
    settingsButton->setMenu( &m_settingsMenu );
    settingsButton->setIcon( QIcon(":/config.png") );
    settingsButton->setPopupMode( QToolButton::InstantPopup );
    m_circToolBar.addWidget( settingsButton );
    m_circToolBar.addSeparator();//..........................

    for( int i=0; i<MaxRecentFiles; i++ ) m_fileMenu.addAction( recentFileActs[i] );
    QToolButton* fileButton = new QToolButton( this );
    fileButton->setToolTip( tr("Last Circuits") );
    fileButton->setMenu( &m_fileMenu );
    fileButton->setIcon( QIcon(":/lastfiles.png") );
    fileButton->setPopupMode( QToolButton::InstantPopup );
    m_circToolBar.addWidget( fileButton );

    m_circToolBar.setObjectName( "m_circToolBar" );
    m_circToolBar.addAction( newCircAct );

    m_circToolBar.addAction( openCircAct );
    m_circToolBar.addAction( saveCircAct );
    m_circToolBar.addAction( saveCircAsAct );
    m_circToolBar.addSeparator();//..........................
    m_circToolBar.addAction( powerCircAct );
    m_circToolBar.addAction( pauseSimAct );
    m_circToolBar.addSeparator();//..........................
    m_circToolBar.addWidget( m_rateLabel );
    //m_circToolBar.addSeparator();
    m_circToolBar.addWidget( m_msgLabel );

    QWidget *spacerWidget = new QWidget( this );
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible( true );
    m_circToolBar.addWidget( spacerWidget );

    m_infoMenu.addAction( infoAct );
    m_infoMenu.addAction( aboutAct );
    m_infoMenu.addAction( aboutQtAct );

    QToolButton* infoButton = new QToolButton( this );
    infoButton->setToolTip( tr("Info") );
    infoButton->setMenu( &m_infoMenu );
    infoButton->setIcon( QIcon(":/help.png") );
    infoButton->setPopupMode( QToolButton::InstantPopup );
    m_circToolBar.addWidget( infoButton );
    
    m_circToolBar.addSeparator();//..........................
}

bool CircuitWidget::newCircuit()
{
    if( Simulator::self() ) powerCircOff();
    
    if( MainWindow::self()->windowTitle().endsWith('*') )
    {
        const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, "MainWindow::closeEvent",
                               tr("\nCircuit has been modified.\n"
                                  "Do you want to save your changes?\n"),
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                               
        if     ( ret == QMessageBox::Save )   saveCirc();
        else if( ret == QMessageBox::Cancel ) return false;
    }
    clear();

    Circuit::self()->setAutoBck( MainWindow::self()->autoBck() );

    m_curCirc = "";
    Simulator::self()->addToUpdateList( &m_outPane );

    MainWindow::self()->setTitle( tr("New Circuit"));
    MainWindow::self()->settings()->setValue( "lastCircDir", m_lastCircDir );
    
    return true;
}

void CircuitWidget::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>( sender() );
    if( action ) loadCirc( action->data().toString() );
}

void CircuitWidget::openCirc()
{
    const QString dir = m_lastCircDir;
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Circuit"), dir,
                                        tr("Circuits (*.sim*);;All files (*.*)"));
    loadCirc( fileName );
}

void CircuitWidget::loadCirc( QString path )
{
    if( !path.isEmpty()
      &&(path.endsWith(".simu") || path.endsWith(".sim1")) )
    {
        newCircuit();
        Circuit::self()->loadCircuit( path );
   
        m_curCirc = path;
        m_lastCircDir = path;
        MainWindow::self()->setTitle(path.split("/").last());

        QSettings* settings = MainWindow::self()->settings();
        settings->setValue( "lastCircDir", m_lastCircDir );

        QStringList files = settings->value("recentCircList").toStringList();
        files.removeAll( m_curCirc );
        files.prepend( m_curCirc );
        while( files.size() > MaxRecentFiles ) files.removeLast();

        settings->setValue("recentCircList", files );
        updateRecentFileActions();

        m_circView.setCircTime( 0 );
}   }

void CircuitWidget::saveCirc()
{
    if( m_curCirc.isEmpty() ) saveCircAs();
    else                      saveCirc( m_curCirc );
}

void CircuitWidget::saveCircAs()
{
    const QString dir = m_lastCircDir;
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save Circuit"), dir,
                                                     tr("Circuits (*.sim*);;All files (*.*)"));
    if( fileName.isEmpty() ) return;

    saveCirc( fileName );
}

void CircuitWidget::saveCirc( QString file )
{
    emit saving(); // Used by SubPackage

    if     (  file.endsWith(".simu") ) file = changeExt( file, ".sim1" );
    else if( !file.endsWith(".sim1") ) file.append(".sim1");

    if( Circuit::self()->saveCircuit( file ) )
    {
        m_curCirc = file;
        m_lastCircDir = file;
        MainWindow::self()->setTitle(file.split("/").last());
        MainWindow::self()->settings()->setValue( "lastCircDir", m_lastCircDir );
    }
    else qDebug() << "\nError Saving Circuit:\n" << file;
}

void CircuitWidget::powerCirc()
{
    if     ( powerCircAct->iconText() == "Off" ) powerCircOn();
    else if( powerCircAct->iconText() == "On" )  powerCircOff();
}

void CircuitWidget::powerCircOn()
{
    powerCircAct->setIcon(QIcon(":/poweron.png"));
    powerCircAct->setEnabled( true );
    powerCircAct->setIconText("On");
    pauseSimAct->setEnabled( true );
    Simulator::self()->startSim();
}

void CircuitWidget::powerCircOff()
{
    if( Simulator::self()->isPaused() ) Simulator::self()->resumeSim();
    if( Simulator::self()->isRunning() ) Simulator::self()->stopSim();

    powerCircAct->setIcon(QIcon(":/poweroff.png"));
    powerCircAct->setIconText("Off");
    powerCircAct->setEnabled( true );
    pauseSimAct->setEnabled( false );
}

void CircuitWidget::powerCircDebug( bool paused )
{
    powerCircAct->setIcon(QIcon(":/powerdeb.png"));
    powerCircAct->setIconText("Debug");
    powerCircAct->setEnabled( true );

    Simulator::self()->startSim( paused );
    if( paused ) m_rateLabel->setText( tr("    Speed: Debugger") );
}

void CircuitWidget::pauseSim()
{
    if( Simulator::self()->simState() > SIM_PAUSED )
    {
        Simulator::self()->pauseSim();
        powerCircAct->setIcon(QIcon(":/poweroff.png"));
        powerCircAct->setIconText("Off");
        //powerCircAct->setEnabled( false );
    }
    else if( Simulator::self()->isPaused() )
    {
        Simulator::self()->resumeSim();
        powerCircAct->setIcon(QIcon(":/poweron.png"));
        powerCircAct->setIconText("On");
        //powerCircAct->setEnabled( true );
}   }

void CircuitWidget::settApp()
{
    if( !m_appPropW )
    {
        m_appPropW = new AppProp( this );
        QPoint p = mapToGlobal( QPoint(50, 50) );
        m_appPropW->move( p.x(), p.y() );
    }
    m_appPropW->show();
}

void CircuitWidget::settCir()
{
    if( !m_cirPropW )
    {
        m_cirPropW = new CircProp( this );
        QPoint p = mapToGlobal( QPoint(50, 50) );
        m_cirPropW->move( p.x(), p.y() );
    }
    m_cirPropW->show();
}

void CircuitWidget::settSim()
{
    if( !m_simPropW )
    {
        m_simPropW = new SimuProp( this );
        QPoint p = mapToGlobal( QPoint(50, 50) );
        m_simPropW->move( p.x(), p.y() );
    }
    m_simPropW->show();
}

void CircuitWidget::openInfo()
{ QDesktopServices::openUrl(QUrl("http://simulide.com")); }

void CircuitWidget::about()
{
    if( !m_about ) m_about = new AboutDialog( this );
    m_about->show();
}

void CircuitWidget::setRate( double rate, int load )
{
    if( rate < 0 ) m_rateLabel->setText( tr("Circuit ERROR!!!") );
    else
    {
        if( (load > 150) || (load < 0) ) load = 0;
        double speed = (double)rate/100;
        QString Srate = QString::number( speed,'f', 2 );
        if( speed < 100 ) Srate = "0"+Srate;
        if( speed < 10 )  Srate = "0"+Srate;
        QString Sload = QString::number( load );
        if( load < 100 ) Sload = "0"+Sload;
        if( load < 10 )  Sload = "0"+Sload;

        m_rateLabel->setText( tr("    Speed: ")+Srate+" %"
                            + tr("    Load: "      )+Sload+" %    ");
}   }

void CircuitWidget::setError( QString error )
{
    setMsg( error, 2 );
    setRate( -1, 0 );
}

void CircuitWidget::setMsg( QString msg, int type )
{
    if     ( type == 0 ) m_msgLabel->setStyleSheet("QLabel { background-color: lightgreen; color: black;  font-weight: bold;}");
    else if( type == 1 ) m_msgLabel->setStyleSheet("QLabel { background-color: orange;     color: white;  font-weight: bold;}");
    else if( type == 2 ) m_msgLabel->setStyleSheet("QLabel { background-color: red;        color: yellow; font-weight: bold;}");
    m_msgLabel->setText( "   "+msg+"   " );
}

void CircuitWidget::updateRecentFileActions()
{
    QSettings* settings = MainWindow::self()->settings();
    QStringList files = settings->value("recentCircList").toStringList();

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

#include "moc_circuitwidget.cpp"
