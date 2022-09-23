/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QGraphicsView>

class Component;
class Circuit;
class SimuProp;
class QPlainTextEdit;

class CircuitView : public QGraphicsView
{
    Q_OBJECT

    public:
        CircuitView( QWidget *parent );
        ~CircuitView();

 static CircuitView* self() { return m_pSelf; }
 
        void clear();
        
        void wheelEvent( QWheelEvent* event );
        void dragMoveEvent( QDragMoveEvent* event );
        void dragEnterEvent( QDragEnterEvent* event );
        void dragLeaveEvent( QDragLeaveEvent* event );

        void mousePressEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );
        
        void setCircTime( uint64_t tStep );

        qreal getScale() { return m_scale; }

    public slots:
        void saveImage();
        void slotPaste();
        void importCirc();
        void zoomToFit();
        
    protected:
        void contextMenuEvent( QContextMenuEvent* event );

    private:
 static CircuitView*  m_pSelf;

        qreal m_scale;
        QString m_help;
        QPlainTextEdit* m_info;
 
        Component*  m_enterItem;
        Circuit*    m_circuit;

        QPointF m_eventpoint;
};

#endif
