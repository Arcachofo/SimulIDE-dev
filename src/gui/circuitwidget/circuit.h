/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QGraphicsScene>
#include <QDomDocument>
#include <QTimer>

#include "component.h"
#include "connector.h"
#include "pin.h"

class Simulator;
class Node;

class MAINMODULE_EXPORT Circuit : public QGraphicsScene
{
    friend class SubCircuit;

    Q_OBJECT

    public:
        Circuit( qreal x, qreal y, qreal width, qreal height, QGraphicsView* parent );
        ~Circuit();

 static Circuit* self() { return m_pSelf; }
        
        bool drawGrid() { return !m_hideGrid; }
        void setDrawGrid( bool draw );
        
        bool showScroll() { return m_showScroll; }
        void setShowScroll( bool show );
        
        bool animate() { return m_animate; }
        void setAnimate( bool an ) { m_animate = an; update(); }

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
        QString getCompId( QString name );
        QString origId( QString name ) { return m_idMap.value( name ); }

        bool is_constarted() { return m_conStarted ; }

        bool  pasting() { return m_pasting; }
        QPointF deltaMove() { return m_deltaMove; }
        
        void addPin( Pin* pin, QString pinId ) { m_pinMap[ pinId ] = pin; }
        void updatePin( ePin* epin, QString newId );
        void removePin( QString pinId ) { m_pinMap.remove( pinId ); }

        const QString getFilePath() const { return m_filePath; }

        QString circType() { return m_circType; }
        double circVersion() { return m_circVersion; }

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
        void keyPressEvent ( QKeyEvent* event );
        void keyReleaseEvent( QKeyEvent* event );
        void dropEvent( QGraphicsSceneDragDropEvent* event );

    private:
        void loadStrDoc( QString doc );
        void loadDomDoc( QDomDocument* doc );
        void loadProperties( QDomElement* element, CompBase* comp );
        void loadCompProperties( QDomElement* element, Component* comp );
        void loadItemProperties( QDomElement* element, Component* comp );
        QString circuitHeader();
        QString circuitToString();
        bool saveString(QString &fileName, QString doc );

        void updatePinName( QString* name );

 static Circuit*  m_pSelf;

        QDomDocument m_domDoc;

        QString m_filePath;
        QString m_backupPath;
        QString m_circType;

        QRect          m_scenerect;
        QGraphicsView* m_graphicView;
        Connector*     new_connector;

        double m_circVersion;

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

        QPointF m_eventpoint;
        QPointF m_deltaMove;

        QList<Component*> m_compList;   // Component list
        QList<Connector*> m_conList;    // Connector list
        QList<Node*> m_nodeList;    // Connector list
        
        QHash<QString, Pin*> m_pinMap;    // Pin list
        QHash<QString, QString> m_idMap;

        QStringList m_undoStack;
        QStringList m_redoStack;

        QTimer m_bckpTimer;

        Simulator* m_simulator;
};

#endif
