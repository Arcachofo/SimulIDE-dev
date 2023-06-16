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

enum stateMode{
    stateNew=1,
    stateAdd=2,
    stateNewAdd=1+2,
    stateSave=4,
    stateAll=1+2+4
};

class CircuitView;
class SubPackage;
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
        void clearCircuit();
        bool deleting() { return m_deleting; }
        void compRemoved( bool removed ) { m_compRemoved = removed; }
        void saveState();
        void unSaveState();
        void addCompState( CompBase* c, QString p, stateMode=stateAll );

        void setChanged();

        void deselectAll();
        void accepKeys( bool a ) { m_acceptKeys = a; }

        Pin* findPin( int x, int y, QString id );

        void loadCircuit( QString fileName );
        bool saveCircuit( QString fileName );

        Component* createItem( QString name, QString id );

        QString newSceneId() { return QString::number(++m_seqNumber); }
        QString newConnectorId() { return QString::number(++m_conNumber); }

        void newconnector( Pin*  startpin );
        void closeconnector( Pin* endpin );
        void deleteNewConnector();
        void updateConnectors();
        Connector* getNewConnector() { return m_newConnector; }

        void addNode( Node* node );

        QList<Component*>* compList() { return &m_compList; }
        QList<Connector*>* conList()  { return &m_conList; }
        QList<Node*>*      nodeList() { return &m_nodeList; }
        QHash<QString, CompBase*>* compMap() { return &m_compMap;}

        Component* getCompById( QString id );
        QString origId( QString name ) { return m_idMap.value( name ); }

        bool is_constarted() { return m_conStarted ; }

        SubPackage* getBoard() { return m_board; }
        void setBoard( SubPackage* b ) { m_board = b; }

        bool pasting() { return m_pasting; }
        bool isBusy()  { return m_busy || m_pasting | m_deleting; }
        bool isSubc()  { return m_createSubc; }

        void addPin( Pin* pin, QString pinId ) { m_pinMap[ pinId ] = pin; m_LdPinMap[ pinId ] = pin; }
        void remPin( QString pinId ) { m_pinMap.remove( pinId ); }
        void updatePin( ePin* epin, QString oldId, QString newId );

        const QString getFilePath() const { return m_filePath; }
        void setFilePath( QString f ) { m_filePath = f; }

        void drawBackground( QPainter* painter, const QRectF &rect );

    signals:
        void keyEvent( QString key, bool pressed );

    public slots:
        void copy( QPointF eventpoint );
        void paste( QPointF eventpoint );
        void undo();
        void redo();
        void clearUndoRedo();
        void importCirc( QPointF eventpoint );
        //void bom();
        void saveBackup();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void keyPressEvent( QKeyEvent* event );
        void keyReleaseEvent( QKeyEvent* event );
        void dropEvent( QGraphicsSceneDragDropEvent* event );

    private:
        struct compState{       // Component State to be restored by Undo/Redo
            QString component;
            QString property;
            QString valStr;
        };
        struct circState{       // Circuit State to be restored by Undo/Redo
            QList<compState> remove;
            QList<compState> create;
            int size() { return remove.size()+create.size(); }
            void clear() { remove.clear(); create.clear(); }
        };

        bool restoreState( circState step );
        void loadStrDoc( QString &doc );
        bool saveString( QString &fileName, QString doc );
        QString circuitHeader();
        QString circuitToString();
        QString getSeqNumber( QString name );
        QString replaceId( QString pinName );

        void updatePinName( QString* name );

 static Circuit*  m_pSelf;

        QString m_filePath;
        QString m_backupPath;

        QRect        m_scenerect;
        CircuitView* m_graphicView;
        Connector*   m_newConnector;
        CompBase*    m_newComp;

        int m_seqNumber;
        int m_conNumber;
        int m_error;
        int m_maxUndoSteps;
        int m_undoIndex;
        int m_redoIndex;

        bool m_pasting;
        bool m_deleting;
        bool m_loading;
        bool m_conStarted;
        bool m_hideGrid;
        bool m_showScroll;
        bool m_compRemoved;
        bool m_animate;
        bool m_changed;
        bool m_busy;
        bool m_undo;
        bool m_redo;
        bool m_acceptKeys;
        bool m_createSubc;

        QPointF m_eventpoint;
        QPointF m_deltaMove;

        QList<Component*> m_compList;   // Component list
        QList<Connector*> m_conList;    // Connector list
        QList<Node*>      m_nodeList;   // Node list

        SubPackage* m_board;

        QHash<QString, Pin*>      m_pinMap;   // Pin Id to Pin*
        QHash<QString, Pin*>      m_LdPinMap; // Pin Id to Pin* while loading/pasting/importing
        QHash<QString, QString>   m_idMap;    // Component seqNumber to new seqNumber (pasting)
        QHash<QString, CompBase*> m_compMap;  // Component Id to Component*

        circState m_circState;
        QList<circState> m_undoStack;
        QList<circState> m_redoStack;

        QTimer m_bckpTimer;

        Simulator* m_simulator;
};

#endif
