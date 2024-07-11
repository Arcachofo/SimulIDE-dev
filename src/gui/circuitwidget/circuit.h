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

#define COMP_STATE_NEW "__COMP_STATE_NEW__"

class creCompDialog;
class CircuitView;
class SubPackage;
class Simulator;
class Node;

class Circuit : public QGraphicsScene
{
    friend class SubCircuit;
    friend class Simulator;

    Q_OBJECT

    public:
        Circuit( int width, int height, CircuitView* parent );
        ~Circuit();

 static Circuit* self() { return m_pSelf; }

        bool drawGrid() { return !m_hideGrid; }
        void setDrawGrid( bool draw );

        bool animate() { return m_animate; }
        void setAnimate( bool an );

        int sceneWidth() { return m_sceneWidth; }
        void setSceneWidth( int w );

        int sceneHeight() { return m_sceneHeight; }
        void setSceneHeight( int h );

        int autoBck();
        void setAutoBck( int secs );

        void removeItems();
        void removeComp( Component* comp) ;
        void compRemoved( bool removed ) { m_compRemoved = removed; }
        void removeNode( Node* node );
        void removeConnector( Connector* conn );
        void clearCircuit();

        //--- Undo/Redo ----------------------------------
        void saveChanges();
        void removeLastUndo();
        void addCompChange(  QString component, QString property, QString undoVal );
        void saveCompChange( QString component, QString property, QString undoVal );
        void beginCircuitBatch();
        void endCircuitBatch();
        void calcCircuitChanges(); // Calculate total changes
        void cancelUndoStep();     // Revert changes done
        void beginUndoStep();      // Record current state
        void endUndoStep();        // Does create/remove
        bool undoRedo() { return m_undo || m_redo; }
        //------------------------------------------------

        void setChanged();

        void accepKeys( bool a ) { m_acceptKeys = a; }

        Pin* findPin( int x, int y, QString id );

        void loadCircuit( QString fileName );
        bool saveCircuit( QString fileName );

        Component* createItem( QString name, QString id, bool map=true );

        QString newSceneId() { return QString::number(++m_seqNumber); }
        QString newConnectorId() { return QString::number(++m_conNumber); }

        void newconnector( Pin* startpin, bool save=true );
        void closeconnector( Pin* endpin, bool save=false );
        void deleteNewConnector();
        //void updateConnectors();
        Connector* getNewConnector() { return m_newConnector; }

        void addNode( Node* node );
        void addComponent( Component* comp );

        QList<Component*>* compList() { return &m_compList; }
        QList<Connector*>* conList()  { return &m_connList; }
        QList<Node*>*      nodeList() { return &m_nodeList; }
        QHash<QString, CompBase*>* compMap() { return &m_compMap;}

        Component* getCompById( QString id );
        QString origId( QString name ) { return m_idMap.value( name ); } // used by Shield

        bool is_constarted() { return m_conStarted ; }

        SubPackage* getBoard() { return m_board; }
        void setBoard( SubPackage* b ) { m_board = b; }

        bool pasting() { return m_pasting; }
        bool isBusy()  { return m_busy || m_pasting | m_deleting; }
        bool isSubc()  { return m_createSubc; }

        void addPin( Pin* pin, QString pinId ) { m_pinMap[ pinId ] = pin; m_LdPinMap[ pinId ] = pin; }
        void remPin( QString pinId ) { m_pinMap.remove( pinId ); }
        void updatePin( ePin* epin, QString oldId, QString newId );

        QString getSeqNumber( QString name );
        QString replaceId( QString pinName );

        const QString getFilePath() const { return m_filePath; }
        void setFilePath( QString f ) { m_filePath = f; }

        bool saveString( QString &fileName, QString doc );
        QString circuitToString();
        //QString circuitToComp( QString category, QString iconData, QString compType );

        bool isComp() { return m_creCompDialog != NULL; }
        QString category() { return m_category; }
        QString compName() { return m_compName; }
        QString compInfo() { return m_compInfo; }

        QString iconData() { return m_iconData; }
        void setIconData( QString id ) { m_iconData = id; }

        //void setConverting( bool c ) { m_converting = c; } // used when converting old Components from xml
        //bool converting() { return m_converting; }

 static QVector<QStringRef> parseProps( QStringRef line );

        int circuitRev() { return m_circRev; }

        void drawBackground( QPainter* painter, const QRectF &rect );

    signals:
        void keyEvent( QString key, bool pressed );

    public slots:
        void copy( QPointF eventpoint );
        void paste( QPointF eventpoint );
        void undo();
        void redo();
        void importCircuit();
        void createComp();
        void cancelComp();
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
 static Circuit*  m_pSelf;

        void loadStrDoc( QString &doc );

        QString circuitHeader();
        void updatePinName( QString* name );

        void setSize( int width, int height );

        QString m_filePath;
        QString m_backupPath;

        QString m_itemType;
        QString m_category;
        QString m_compName;
        QString m_compInfo;
        QString m_iconData;

        QRect        m_scenerect;
        CircuitView* m_graphicView;
        Connector*   m_newConnector;

        int m_sceneWidth;
        int m_sceneHeight;

        int m_circRev;
        int m_seqNumber;
        int m_conNumber;
        int m_error;

        bool m_pasting;
        bool m_deleting;
        bool m_loading;
        bool m_conStarted;
        bool m_hideGrid;

        bool m_compRemoved;
        bool m_animate;
        bool m_changed;
        bool m_busy;
        bool m_undo;
        bool m_redo;
        bool m_acceptKeys;
        bool m_createSubc;
        int m_cicuitBatch;

        QPointF m_eventpoint;
        QPointF m_deltaMove;

        QList<Component*> m_compList;   // Component list
        QList<Connector*> m_connList;   // Connector list
        QList<Node*>      m_nodeList;   // Node list

        SubPackage* m_board;

        QHash<QString, Pin*>      m_pinMap;   // Pin Id to Pin*
        QHash<QString, Pin*>      m_LdPinMap; // Pin Id to Pin* while loading/pasting/importing
        QHash<QString, QString>   m_idMap;    // Component seqNumber to new seqNumber (pasting)
        QHash<QString, CompBase*> m_compMap;  // Component Id to Component*

        QTimer m_bckpTimer;

        Simulator* m_simulator;

        creCompDialog* m_creCompDialog;

        //--- Undo/Redo ----------------------------------

        struct compChange{      // Component Change to be performed by Undo/Redo to complete a Circuit change
            QString component;  // Component name
            QString property;   // Property name
            QString undoValue;  // Property value for Undo step
            QString redoValue;  // Property value for Redo step
        };
        struct circChange{       // Circuit Change to be performed by Undo/Redo to restore circuit state
            QList<compChange> compChanges;
            int size() { return compChanges.size(); }
            void clear() { compChanges.clear(); }
        };

        inline void clearCircChanges() { m_circChange.clear(); }
        void deleteRemoved();
        void restoreState();

        int m_maxUndoSteps;
        int m_undoIndex;

        circChange m_circChange;
        QList<circChange> m_undoStack;

        QList<CompBase*>  m_removedComps; // removed component list;
        QList<Connector*> m_oldConns;
        QList<Component*> m_oldComps;
        QList<Node*>      m_oldNodes;
        QMap<CompBase*, QString> m_compStrMap;
};

#endif
