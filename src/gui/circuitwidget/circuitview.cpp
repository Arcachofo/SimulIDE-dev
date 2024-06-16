/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include <QSvgGenerator>
#include <QMimeData>
#include <QFileDialog>
#include <QSettings>
#include <QGuiApplication>
#include <QScrollBar>
#include <QTreeWidgetItem>

#include "circuitview.h"
#include "circuitwidget.h"
#include "circuit.h"
#include "componentlist.h"
#include "mainwindow.h"
#include "component.h"
#include "subcircuit.h"
#include "utils.h"
#include "e-diode.h"
#include "linker.h"

#define tr(str) QCoreApplication::translate("CircuitView",str)

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
    setShowScroll( scrollBars );

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
    if( m_circuit ){
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

void CircuitView::setShowScroll( bool show )
{
    m_showScroll = show;
    if( show ){
        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "true" );
    }else{
        setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "false" );
}   }

void CircuitView::wheelEvent( QWheelEvent* event )
{
    qreal scaleFactor = pow( 2.0, event->delta() / 700.0);
    scale( scaleFactor, scaleFactor );
    m_scale *= scaleFactor;
}

void CircuitView::dragEnterEvent( QDragEnterEvent* event )
{
    m_enterItem = NULL;

    QString text  = event->mimeData()->text();

    if( text.startsWith( "file://" ) )
    {
        QGraphicsView::dragEnterEvent( event );
        return;
    }

    QStringList data = text.split(",");
    if( data.size() != 2 ) return;

    QString type = data.last();
    QString name = data.first();

    if( type.isEmpty() || name.isEmpty() ) return;

    event->accept(); // Not moving items in the list (this will prevent removing items from the list)

    m_enterItem = m_circuit->createItem( type, name+"-"+m_circuit->newSceneId() );
    if( m_enterItem )
    {
        m_circuit->clearSelection();
        m_circuit->addComponent( m_enterItem );
        m_circuit->saveCompChange( m_enterItem->getUid(), COMP_STATE_NEW, "" );

        m_enterItem->setPos( mapToScene( event->pos() ) );
        m_enterItem->setSelected( true );

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
    if( !m_enterItem ) return;

    m_circuit->removeComp( m_enterItem );
    Circuit::self()->removeLastUndo();
    m_enterItem = NULL;
}

void CircuitView::mousePressEvent( QMouseEvent* event )
{
    m_waitForDragStart = false;

    if( event->button()   == Qt::LeftButton
     && event->modifiers() & Qt::ControlModifier
     && event->modifiers() & Qt::ShiftModifier
     && !m_circuit->is_constarted() )                   // Prepare Copy by drag
    {
        QGraphicsItem* item = itemAt( event->pos() );
        if( item ){                                                 // Check if item exists before start drag:
            while ( item->parentItem() ) item = item->parentItem(); // Make sure the item deselected is the top graphic item.

            if( !item->isSelected() ) m_circuit->clearSelection();  // If the comp is not selected when drag starts, clear all the selections.
            else                      item->setSelected( false );
            m_mousePressPos = event->pos();
            m_waitForDragStart = true;
        }
    }
    else if( event->button() == Qt::MidButton )
    {
        event->accept();
        setDragMode( QGraphicsView::ScrollHandDrag );

        QGraphicsView::mousePressEvent( event );
        if( event->isAccepted() ) return;

        event = new QMouseEvent( QEvent::MouseButtonPress, event->pos(),
                                 Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
    }
    QGraphicsView::mousePressEvent( event );
}

void CircuitView::mouseMoveEvent( QMouseEvent* event )
{
    if( m_waitForDragStart )
    {
        m_waitForDragStart = false;

        if( event->modifiers() & Qt::ControlModifier
         && event->modifiers() & Qt::ShiftModifier
         && !m_circuit->selectedItems().isEmpty() )     // Start Copy by drag
        {
            event->accept();
            m_circuit->beginCircuitBatch();     // Do this to force paste() to not save changes
            m_circuit->copy( m_mousePressPos ); // Component move after paste() will save changes
            m_circuit->paste( event->pos() );   // paste() will call beginUndoStep()
        }
    }
    QGraphicsView::mouseMoveEvent( event );
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
    if( Linker::m_selecComp ){ // Cancel link to components
        Linker::stopLinking();
        return;
    }
    QGuiApplication::restoreOverrideCursor();
    QGraphicsView::contextMenuEvent( event );

    if( m_circuit->is_constarted() ) m_circuit->deleteNewConnector();
    else if( !event->isAccepted() )
    {
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

        //if( !m_circuit->getFilePath().isEmpty() && !m_circuit->isComp() )
        {
            QAction* createCompAct = menu.addAction(QIcon(":/subc.png"), tr("Create Component") );
            connect( createCompAct, &QAction::triggered,
                       m_circuit, &Circuit::createComp, Qt::UniqueConnection );
        }

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

void CircuitView::importCirc() { Circuit::self()->importCircuit(); }

void CircuitView::slotPaste() { m_circuit->paste( m_eventpoint ); }

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
}   }   }
