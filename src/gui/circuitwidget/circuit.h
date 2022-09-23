/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QGraphicsScene>
#include <QTimer>

#include "component.h"
#include "connector.h"
#include "pin.h"

class CircuitView;
class Simulator;
class Node;

class MAINMODULE_EXPORT Circuit : public QGraphicsScene
{
    friend class SubCircuit;
    friend class Simulator;

    Q_OBJECT

    public:
        Circuit( qreal x, qreal y, qreal width, qreal height, CircuitView* parent );
        ~Circuit();

 static Circuit* self() { return m_pSelf; }

        bool drawGrid() { return !m_hideGrid; }
        void setDrawGrid( bool draw );
        
        bool showScroll() { return m_showScroll; }
        void setShowScroll( bool show );
        
        bool animate() { return m_animate; }
        void setAnimate( bool an );

        int autoBck();
        void setAutoBck( int secs );
        
        void removeItems();
        void removeComp( Component* comp );
        void remove();
        bool deleting() { return m_deleting; }
        void compRemoved( bool removed ) { m_compRemoved = removed; }
        void saveState();
        void unSaveState() { m_undoStack.takeLast(); }
        void setChanged() { m_changed = true; }

        void deselectAll();
        void accepKeys( bool a ) { m_acceptKeys = a; }

        void drawBackground( QPainter* painter, const QRectF &rect );

        Pin* findPin( int x, int y, QString id );

        void loadCircuit( QString fileName );
        bool saveCircuit( QString fileName );

        Component* createItem( QString name, QString id );

        QString newSceneId() { return QString("%1").arg(++m_seqNumber) ; }

        void newconnector( Pin*  startpin );
        void closeconnector( Pin* endpin );
        void deleteNewConnector();
        void updateConnectors();
        Connector* getNewConnector() { return new_connector; }

        void addNode( Node* node );

        QList<Component*>* compList() { return &m_compList; }
        QList<Connector*>* conList()  { return &m_conList; }
        QList<Node*>*      nodeList()  { return &m_nodeList; }

        Component* getCompById( QString id );
        QString origId( QString name ) { return m_idMap.value( name ); }

        bool is_constarted() { return m_conStarted ; }

        bool pasting() { return m_pasting; }
        bool isBusy() { return m_busy || m_pasting | m_deleting; }
        
        void addPin( Pin* pin, QString pinId ) { m_pinMap[ pinId ] = pin; m_LdPinMap[ pinId ] = pin; }
        void remPin( QString pinId ) { m_pinMap.remove( pinId ); }
        void updatePin( ePin* epin, QString oldId, QString newId );

        const QString getFilePath() const { return m_filePath; }

    signals:
        void keyEvent( QString key, bool pressed );

    public slots:
        void copy( QPointF eventpoint );
        void paste( QPointF eventpoint );
        void undo();
        void redo();
        void importCirc( QPointF eventpoint );
        void bom();
        void saveChanges();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void keyPressEvent( QKeyEvent* event );
        void keyReleaseEvent( QKeyEvent* event );
        void dropEvent( QGraphicsSceneDragDropEvent* event );

    private:
        QString getCompId( QString &pinName );
        void loadStrDoc( QString &doc );
        QString circuitHeader();
        QString circuitToString();
        bool saveString(QString &fileName, QString doc );

        void updatePinName( QString* name );

 static Circuit*  m_pSelf;

        QString m_filePath;
        QString m_backupPath;

        QRect        m_scenerect;
        CircuitView* m_graphicView;
        Connector*   new_connector;

        int m_seqNumber;
        int m_error;

        bool m_pasting;
        bool m_deleting;
        bool m_conStarted;
        bool m_hideGrid;
        bool m_showScroll;
        bool m_compRemoved;
        bool m_animate;
        bool m_changed;
        bool m_busy;
        bool m_acceptKeys;

        QPointF m_eventpoint;
        QPointF m_deltaMove;

        QList<Component*> m_compList;   // Component list
        QList<Connector*> m_conList;    // Connector list
        QList<Node*> m_nodeList;        // Node list
        
        QHash<QString, Pin*> m_pinMap;    // Pin list
        QHash<QString, Pin*> m_LdPinMap;  // Pin list while loading/pasting/importing
        QHash<QString, QString> m_idMap;

        QStringList m_undoStack;
        QStringList m_redoStack;

        QTimer m_bckpTimer;

        Simulator* m_simulator;
};

#endif
