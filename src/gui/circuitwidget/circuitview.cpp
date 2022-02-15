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

#include <QSvgGenerator>
#include <QMimeData>
#include <QFileDialog>
#include <QSettings>

#include "circuitview.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "mainwindow.h"
#include "component.h"
#include "mcubase.h"
#include "subcircuit.h"
#include "utils.h"
#include "e-diode.h"

CircuitView*  CircuitView::m_pSelf = 0l;

CircuitView::CircuitView( QWidget *parent )
           : QGraphicsView( parent )
{
    setObjectName( "CircuitView" );
    m_pSelf = this;

    m_scale = 1;
    m_help = "";
    m_circuit   = NULL;
    m_enterItem = NULL;

    eDiode::getModels();

    //viewport()->setFixedSize( 3200, 2400 );
    bool scrollBars = MainWindow::self()->settings()->value( "Circuit/showScroll" ).toBool();
    if( scrollBars )
    {
        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    }else{
        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    }
    //setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    //setCacheMode( CacheBackground );
    //setRenderHint( QPainter::Antialiasing );
    setRenderHints( QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setTransformationAnchor( AnchorUnderMouse );
    setResizeAnchor( AnchorUnderMouse );
    setDragMode( RubberBandDrag );

    setAcceptDrops(true);
    
    m_info = new QPlainTextEdit( this );
    m_info->setObjectName( "m_info" );
    m_info->setLineWrapMode( QPlainTextEdit::NoWrap );
    m_info->setPlainText( "Time: 00:00:00.000000" );
    m_info->setWindowFlags( Qt::FramelessWindowHint );
    m_info->setAttribute( Qt::WA_NoSystemBackground );
    m_info->setAttribute( Qt::WA_TranslucentBackground );
    m_info->setAttribute( Qt::WA_TransparentForMouseEvents );
    m_info->setStyleSheet( "color: #884433;background-color: rgba(0,0,0,0)" );
    m_info->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff  );
    m_info->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff  );
    
    double fontScale = MainWindow::self()->fontScale();
    QFont font = m_info->font();
    font.setBold( true );
    font.setPixelSize( int(10*fontScale) );
    m_info->setFont( font );
    
    //m_info->setMaximumSize( 500*fontScale, 80*fontScale );
    m_info->setMaximumHeight( 40*fontScale );
    m_info->setMinimumSize( 500*fontScale, 40*fontScale );
    m_info->show();
}
CircuitView::~CircuitView() { }

void CircuitView::setCircTime( uint64_t tStep )
{
    double step = tStep/1e6;
    int hours = step/3600e6;
    step -= hours*3600e6;
    int mins  = step/60e6;
    step -= mins*60e6;
    int secs  = step/1e6;
    step -= secs*1e6;
    int mSecs = step/1e3;
    step -= mSecs*1e3;
    int uSecs = step;
    step -= uSecs;
    int nSecs = step*1e3;
    step -= nSecs/1e3;
    step += 1e-7;
    int pSecs = step*1e6;

    QString strH = QString::number( hours );
    if( strH.length() < 2 ) strH = "0"+strH;
    QString strM = QString::number( mins );
    if( strM.length() < 2 ) strM = "0"+strM;
    QString strS = QString::number( secs );
    if( strS.length() < 2 ) strS = "0"+strS;
    QString strMS = QString::number( mSecs );
    while( strMS.length() < 3 ) strMS = "0"+strMS;
    QString strUS = QString::number( uSecs );
    while( strUS.length() < 3 ) strUS = "0"+strUS;
    QString strNS = QString::number( nSecs );
    while( strNS.length() < 3 ) strNS = "0"+strNS;
    QString strPS = QString::number( pSecs );
    while( strPS.length() < 3 ) strPS = "0"+strPS;
    
    QString strMcu = " ";
    
    if( McuBase::self() )
    {
        QString device = McuBase::self()->device();
        QString freq = QString::number( McuBase::self()->freq()*1e-6 );
        strMcu = "      Mcu: "+device+" at "+freq+" MHz";
    }
    m_info->setPlainText( tr("Time: ")+strH+":"+strM+":"+strS+" s  "
                          +strMS+" ms  "+strUS+" µs  "+strNS+" ns  "+strPS+" ps "+strMcu );
}

void CircuitView::clear()
{
    if( m_circuit ) 
    {
        m_circuit->remove();
        m_circuit->deleteLater();
    }
    m_circuit = new Circuit( -1600, -1200, 3200, 2400, this );
    setScene( m_circuit );
    resetMatrix();
    m_scale = 1;
    m_enterItem = NULL;
    centerOn( 900, 600 );
}

void CircuitView::wheelEvent( QWheelEvent* event )
{
    qreal scaleFactor = pow( 2.0, event->delta() / 700.0);
    scale( scaleFactor, scaleFactor );
    m_scale *= scaleFactor;
}

void CircuitView::dragEnterEvent( QDragEnterEvent* event )
{
    event->accept();
    m_enterItem = NULL;

    QString type = event->mimeData()->html();
    QString name = event->mimeData()->text();

    if( type.isEmpty() || name.isEmpty() ) return;
    Circuit::self()->saveState();

    m_enterItem = m_circuit->createItem( type, name+"-"+m_circuit->newSceneId() );
    if( m_enterItem )
    {
        if( type == "Subcircuit" )
        {
            SubCircuit* subC = static_cast<SubCircuit*>( m_enterItem );
            subC->setLogicSymbol( true );
        }
        m_enterItem->setPos( mapToScene( event->pos() ) );
        m_circuit->addItem( m_enterItem );
        m_circuit->compList()->append( m_enterItem );
        this->setFocus();
    }
    else Circuit::self()->unSaveState();
}

void CircuitView::dragMoveEvent( QDragMoveEvent* event )
{
    event->accept();
    if( m_enterItem ) m_enterItem->moveTo( togrid( mapToScene( event->pos() ) ) );
}

void CircuitView::dragLeaveEvent( QDragLeaveEvent* event )
{
    event->accept();
    if ( m_enterItem )
    {
        m_circuit->removeComp( m_enterItem );
        m_enterItem = NULL;
        Circuit::self()->unSaveState();
}   }

void CircuitView::mousePressEvent( QMouseEvent* event )
{
    if( event->button() == Qt::MidButton )
    {
        event->accept();
        setDragMode( QGraphicsView::ScrollHandDrag );

        QGraphicsView::mousePressEvent( event );

        if( !event->isAccepted() )
        {
            QMouseEvent eve( QEvent::MouseButtonPress, event->pos(),
                Qt::LeftButton, Qt::LeftButton, Qt::NoModifier   );
            QGraphicsView::mousePressEvent( &eve );
    }   }
    else QGraphicsView::mousePressEvent( event );
}

void CircuitView::mouseReleaseEvent( QMouseEvent* event )
{
    if( event->button() == Qt::MidButton )
    {
        event->accept();
        QMouseEvent eve( QEvent::MouseButtonRelease, event->pos(),
            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier   );

        QGraphicsView::mouseReleaseEvent( &eve );
    }
    else QGraphicsView::mouseReleaseEvent( event );

    viewport()->setCursor( Qt::ArrowCursor );
    setDragMode( QGraphicsView::RubberBandDrag );
}

void CircuitView::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent( event );

    if( m_circuit->is_constarted() ) m_circuit->deleteNewConnector();
    else if( !event->isAccepted() )
    {
        QPointF eventPos = mapToScene( event->globalPos() ) ;
        m_eventpoint = mapToScene( event->pos()  );

        QMenu menu;

        QAction* zoomToFitAction = menu.addAction( QIcon(":/zoomfit.png"),tr("Zoom to Fit") );
                connect( zoomToFitAction, SIGNAL( triggered()),
                                    this, SLOT( zoomToFit() ), Qt::UniqueConnection  );

        menu.addSeparator();

        QAction* pasteAction = menu.addAction(QIcon(":/paste.png"),tr("Paste")+"\tCtrl+V");
        connect( pasteAction, SIGNAL( triggered()),
                        this, SLOT(slotPaste()), Qt::UniqueConnection );

        QAction* undoAction = menu.addAction(QIcon(":/undo.png"),tr("Undo")+"\tCtrl+Z");
        connect( undoAction, SIGNAL( triggered()),
                  m_circuit, SLOT(undo()), Qt::UniqueConnection );

        QAction* redoAction = menu.addAction(QIcon(":/redo.png"),tr("Redo")+"\tCtrl+Y");
        connect( redoAction, SIGNAL( triggered()),
                  m_circuit, SLOT(redo()), Qt::UniqueConnection );

        menu.addSeparator();

        QAction* importCircAct = menu.addAction(QIcon(":/opencirc.png"), tr("Import Circuit") );
        connect(importCircAct, SIGNAL(triggered()),
                         this, SLOT(importCirc()), Qt::UniqueConnection );

        QAction* saveImgAct = menu.addAction( QIcon(":/saveimage.png"), tr("Save Circuit as Image") );
        connect( saveImgAct, SIGNAL(triggered()),
                       this, SLOT(saveImage()), Qt::UniqueConnection );

        QAction* createBomAct = menu.addAction(QIcon(":/savecirc.png"), tr("Bill of Materials") );
        connect(createBomAct, SIGNAL(triggered()),
                   m_circuit, SLOT( bom() ), Qt::UniqueConnection );

        menu.exec( mapFromScene( eventPos ) );
}   }

void CircuitView::zoomToFit()
{
    QRectF r = m_circuit->itemsBoundingRect();
    fitInView( r, Qt::KeepAspectRatio );
}

void CircuitView::importCirc()
{ Circuit::self()->importCirc( m_eventpoint ); }

void CircuitView::slotPaste()
{ Circuit::self()->paste( m_eventpoint ); }

void CircuitView::saveImage()
{
    QString circPath = changeExt( Circuit::self()->getFilePath(), ".png" );
    
    QString fileName = QFileDialog::getSaveFileName( this
                            , tr( "Save as Image" )
                            , circPath
                            , "PNG (*.png);;JPEG (*.jpeg);;BMP (*.bmp);;SVG (*.svg);;All (*.*)"  );
    if( !fileName.isNull() )
    {
        if( fileName.endsWith( ".svg" ) )
        {
            QSvgGenerator svgGen;

            svgGen.setFileName( fileName );
            svgGen.setSize( QSize(3200, 2400) );
            svgGen.setViewBox( QRect(0, 0, 3200, 2400) );
            svgGen.setTitle( tr("Circuit Name") );
            svgGen.setDescription( tr("Generated by SimulIDE") );

            QPainter painter( &svgGen );
            Circuit::self()->render( &painter );
        }else{
            QPixmap pixMap = this->grab();
            pixMap.save( fileName );
}   }    }

#include "moc_circuitview.cpp"
