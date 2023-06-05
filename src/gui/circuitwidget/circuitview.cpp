/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QSvgGenerator>
#include <QMimeData>
#include <QFileDialog>
#include <QSettings>
#include <QGuiApplication>

#include "circuitview.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "mainwindow.h"
#include "component.h"
#include "subcircuit.h"
#include "utils.h"
#include "e-diode.h"
#include "linkable.h"

CircuitView*  CircuitView::m_pSelf = NULL;

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
}
CircuitView::~CircuitView() { }

void CircuitView::clear()
{
    if( m_circuit ) 
    {
        m_circuit->clearCircuit();
        m_circuit->deleteLater();
    }
    m_circuit = new Circuit( -1600, -1200, 3200, 2400, this );
    setScene( m_circuit );
    resetMatrix();
    m_scale = 1;
    m_enterItem = NULL;
    centerOn( 0, 0 );
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

    m_enterItem = m_circuit->createItem( type, name+"-"+m_circuit->newSceneId() );
    if( m_enterItem )
    {
        if( type == "Subcircuit" )
        {
            SubCircuit* subC = static_cast<SubCircuit*>( m_enterItem );
            if( subC->subcType() < Chip::Board ) subC->setLogicSymbol( true );
        }
        m_enterItem->setPos( mapToScene( event->pos() ) );
        m_circuit->addItem( m_enterItem );
        m_circuit->compList()->append( m_enterItem );
        m_circuit->addCompState( m_enterItem, "remove" );
        this->setFocus();
    }
}

void CircuitView::dragMoveEvent( QDragMoveEvent* event )
{
    event->accept();
    if( m_enterItem ) m_enterItem->moveTo( toGrid( mapToScene( event->pos() ) ) );
}

void CircuitView::dragLeaveEvent( QDragLeaveEvent* event )
{
    event->accept();
    if ( m_enterItem )
    {
        m_circuit->removeComp( m_enterItem );
        Circuit::self()->unSaveState();
        m_enterItem = NULL;
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
        setDragMode( QGraphicsView::RubberBandDrag );
    }
    else QGraphicsView::mouseReleaseEvent( event );
}

void CircuitView::overrideCursor( const QCursor &cursor )
{
    setDragMode( QGraphicsView::ScrollHandDrag );
    setCursor( cursor );
    setDragMode( QGraphicsView::RubberBandDrag );
}

void CircuitView::contextMenuEvent( QContextMenuEvent* event )
{
    QGuiApplication::restoreOverrideCursor();
    QGraphicsView::contextMenuEvent( event );

    if( m_circuit->is_constarted() ) m_circuit->deleteNewConnector();
    else if( !event->isAccepted() )
    {
        if( Component::m_selecComp ) // Cancel link to components
        {
            Component::m_selecComp->compSelected( NULL );
            return;
        }

        QPointF eventPos = mapToScene( event->globalPos() ) ;
        m_eventpoint = mapToScene( event->pos()  );

        QMenu menu;

        menu.addSeparator();

        QAction* pasteAction = menu.addAction(QIcon(":/paste.svg"),tr("Paste")+"\tCtrl+V");
        connect( pasteAction, &QAction::triggered,
                        this, &CircuitView::slotPaste, Qt::UniqueConnection );

        QAction* undoAction = menu.addAction(QIcon(":/undo.svg"),tr("Undo")+"\tCtrl+Z");
        connect( undoAction, &QAction::triggered,
                  m_circuit, &Circuit::undo, Qt::UniqueConnection );

        QAction* redoAction = menu.addAction(QIcon(":/redo.svg"),tr("Redo")+"\tCtrl+Y");
        connect( redoAction, &QAction::triggered,
                  m_circuit, &Circuit::redo, Qt::UniqueConnection );

        menu.addSeparator();

        QAction* importCircAct = menu.addAction(QIcon(":/open.svg"), tr("Import Circuit") );
        connect(importCircAct, &QAction::triggered,
                         this, &CircuitView::importCirc, Qt::UniqueConnection );

        QAction* saveImgAct = menu.addAction( QIcon(":/saveimage.svg"), tr("Save Circuit as Image") );
        connect( saveImgAct, &QAction::triggered,
                       this, &CircuitView::saveImage, Qt::UniqueConnection );

        /*QAction* createBomAct = menu.addAction(QIcon(":/savecirc.png"), tr("Bill of Materials") );
        connect(createBomAct, &QAction::triggered,
                   m_circuit, &Circuit::bom, Qt::UniqueConnection );*/

        menu.exec( mapFromScene( eventPos ) );
}   }

void CircuitView::zoomToFit()
{
    QRectF r = m_circuit->itemsBoundingRect();
    fitInView( r, Qt::KeepAspectRatio );
}

void CircuitView::zoomSelected()
{
    QRectF r;
    for( QGraphicsItem *item : m_circuit->items() )
      if( item->isSelected() ) r |= item->sceneBoundingRect();

    fitInView( r, Qt::KeepAspectRatio );
}

void CircuitView::zoomOne()
{
    resetMatrix();
    m_scale = 1;
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
            svgGen.setTitle("Circuit Name");
            svgGen.setDescription("Generated by SimulIDE");

            QPainter painter( &svgGen );
            Circuit::self()->render( &painter );
        }else{
            QPixmap pixMap = this->grab();
            pixMap.save( fileName );
}   }    }
