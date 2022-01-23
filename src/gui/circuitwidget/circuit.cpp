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

#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>
#include <QSettings>

#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "connectorline.h"
#include "node.h"
#include "utils.h"
#include "subcircuit.h"
#include "mcu.h"
#include "simulator.h"
#include "e-node.h"

Circuit* Circuit::m_pSelf = NULL;

Circuit::Circuit( qreal x, qreal y, qreal width, qreal height, QGraphicsView*  parent )
       : QGraphicsScene( x, y, width, height, parent )
{
    m_simulator = new Simulator();
    
    setObjectName( "Circuit" );
    setParent( parent );
    m_graphicView = parent;
    m_scenerect.setRect( x, y, width, height );
    setSceneRect( QRectF(x, y, width, height) );

    m_pSelf = this;
    m_circType = "simulide_1.0";
    m_circVersion = 1.0;

    m_busy       = false;
    m_changed    = false;
    m_animate    = false;
    m_pasting    = false;
    m_deleting   = false;
    m_conStarted = false;

    new_connector = NULL;
    m_seqNumber   = 0;

    m_backupPath = MainWindow::self()->getConfigPath("backup.sim1");
    m_hideGrid   = MainWindow::self()->settings()->value( "Circuit/hideGrid" ).toBool();
    m_showScroll = MainWindow::self()->settings()->value( "Circuit/showScroll" ).toBool();
    m_filePath   = qApp->applicationDirPath()+"/new.simu";

    connect( &m_bckpTimer, SIGNAL(timeout() ),
                     this, SLOT( saveChanges()), Qt::UniqueConnection );
}

Circuit::~Circuit()
{
    delete m_simulator;

    m_bckpTimer.stop();
    m_undoStack.clear();
    m_redoStack.clear();

    QFile file( m_backupPath );
    if( !file.exists() ) return;
    QFile::remove( m_backupPath ); // Remove backup file
}

Component* Circuit::getCompById( QString id )
{
    for( Component* comp : m_compList ) if( comp->objectName() == id ) return comp;
    return NULL;
}

QString Circuit::getCompId( QString &pinName )
{
    QStringList list = pinName.split("-");
    if( list.size() < 2 ) return "";
    //QString pinId  = nameSplit.takeLast();
    QString compId = list.at( 0 )+"-"+list.at(1);

    return compId;
}

Pin* Circuit::findPin( int x, int y, QString id )
{
    /// qDebug() << "Circuit::findPin" << id;
    QRectF itemRect = QRectF ( x-4, y-4, 8, 8 );

    QList<QGraphicsItem*> list = items( itemRect ); // List of items in (x, y)
    for( QGraphicsItem* it : list )
    {
        Pin* pin =  qgraphicsitem_cast<Pin*>( it );
        if( pin && (pin->pinId().left(1) == id.left(1))) return pin; // Check if names start by same letter
    }
    for( QGraphicsItem* it : list ) // Not found by first letter, take first Pin
    {
        Pin* pin =  qgraphicsitem_cast<Pin*>( it );
        if( pin ) return pin;
    }
    return NULL;
}

void Circuit::loadCircuit( QString fileName )
{
    if( m_conStarted ) return;
    saveState();

    m_filePath = fileName;
    m_error = 0;

    QString doc = fileToString( fileName, "Circuit::loadCircuit" );
    loadStrDoc( doc );

    if( m_error != 0 ) 
    {
        remove();
        for( Node* nod : m_nodeList ) removeComp( nod ); // Clean Nodes
    }else{
        m_graphicView->centerOn( QPointF(
            width()/2  + itemsBoundingRect().center().x() - m_graphicView->width()/2,
            height()/2 + itemsBoundingRect().center().y() - m_graphicView->height()/2 ) );
        qDebug() << "Circuit Loaded: ";
        qDebug() << fileName;
        //this->addRect( itemsBoundingRect() );
}   }

void Circuit::loadStrDoc( QString &doc )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Component*> compList;   // Pasting Component list
    QList<Connector*> conList;    // Pasting Connector list
    QList<Node*> nodeList;        // Pasting node list

    Component* lastComp = NULL;
    QList<SubCircuit*> shieldList;
    //QHash<QString, eNode*> nodMap;
    m_busy    = true;
    m_LdPinMap.clear();

    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.startsWith( "<circuit") && !m_pasting )
        {
            line = line.mid( 9, line.length()-11 );
            QVector<QStringRef> props = line.split(" ");
            for( QStringRef prop : props )
            {
                QVector<QStringRef> p = prop.split("=");
                if( p.size() != 2 ) continue;

                QStringRef name = p.first(), val = p.last();
                val = val.mid( 1, val.lastIndexOf("\"")-1 ); // Remove "

                if     ( name == "stepsPS" ) m_simulator->setStepsPerSec( val.toDouble() );
                else if( name == "NLsteps" ) m_simulator->setMaxNlSteps( val.toUInt() );
                else if( name == "stepSize") m_simulator->setStepSize( val.toULongLong() );
                else if( name == "animate" ) setAnimate( val.toInt() );
            }
        }
        else if( line.contains("<mainCompProps") )
        {
            if( !lastComp ) continue;
            SubCircuit* subci = static_cast<SubCircuit*>(lastComp);
            Component* mComp = subci->getMainComp();
            if( !mComp ) continue;

            QString propName = "";
            QVector<QStringRef> props = line.split("\"");
            for( QStringRef prop : props )
            {
                if( prop.endsWith("=") )
                {
                    prop = prop.split(" ").last();
                    propName = prop.toString().mid( 0, prop.length()-1 );
                    continue;
                }
                else if( prop.endsWith("/>") ) continue;
                if( !mComp->setPropStr( propName, prop.toString() ) ){
                    if( propName.toLower()  != "tristate" )               // TODELETE
                        qDebug() << "Circuit: Wrong Property: "<<mComp->itemType()<<propName; }

                propName = "";
            }
        }
        else if( line.startsWith("<item") )
        {
            QString uid, newUid, type, label;

            QStringRef name;
            QVector<QStringRef> props = line.split("\"");
            QVector<QStringRef> properties;
            for( QStringRef prop : props )
            {
                if( prop.endsWith("=") )
                {
                    prop = prop.split(" ").last();
                    name = prop.mid( 0, prop.length()-1 );
                    continue;
                }
                else if( prop.endsWith("/>") ) continue;
                else{
                    if     ( name == "itemtype"   ) type  = prop.toString();
                    else if( name == "CircId"     ) uid   = prop.toString();
                    else if( name == "objectName" ) uid   = prop.toString();
                    else if( name == "label"      ) label = prop.toString();
                    else if( name == "id"         ) label = prop.toString();
                    else properties << name << prop ;
            }   }
            if( type.isEmpty() ) { qDebug() << "ERROR: Component with no type:"<<label<< uid; continue;}

            if( m_pasting ) // Create new id
            {
                if( type == "Subcircuit" || type == "MCU" )
                     newUid = uid.split("-").first()+"-"+newSceneId();
                else newUid = type+"-"+newSceneId();
            }
            else     newUid = uid;

            if( type == "Connector" )
            {
                Pin* startpin = NULL;
                Pin* endpin   = NULL;
                QString startpinid, endpinid;
                QStringList pointList;

                QString name = "";
                for( QStringRef prop : properties )
                {
                    if( name.isEmpty() ) { name = prop.toString(); continue; }

                    if     ( name == "startpinid") startpinid = prop.toString();
                    else if( name == "endpinid"  ) endpinid   = prop.toString();
                    else if( name == "pointList" ) pointList  = prop.toString().split(",");
                    else if( name == "uid"       ) uid   = prop.toString();
                    name = "";
                }
                if( m_pasting )
                {
                    QString startCompId = getCompId( startpinid );
                    QString endCompId   = getCompId( endpinid );

                    startpinid.replace( startCompId, m_idMap.value(startCompId) );
                    endpinid.replace(   endCompId,   m_idMap.value(endCompId) );
                }
                startpin = m_LdPinMap.value( startpinid );
                endpin   = m_LdPinMap.value( endpinid );

                if( !startpin ) // Pin not found by name... find it by pos
                {
                    int itemX = pointList.first().toInt();
                    int itemY = pointList.at(1).toInt();
                    if( !m_pasting) startpin = findPin( itemX, itemY, startpinid );
                }
                if( !endpin ) // Pin not found by name... find it by pos
                {
                    int itemX = pointList.at( pointList.size()-2 ).toInt();
                    int itemY = pointList.last().toInt();
                    if( !m_pasting) endpin = findPin( itemX, itemY, endpinid );
                }
                if( startpin && startpin->connector() ) startpin = NULL;
                if( endpin   && endpin->connector() )   endpin   = NULL;

                if( startpin && endpin )    // Create Connector
                {
                    if( uid.isEmpty() ) uid = "connector"+newSceneId();
                    Connector* con = new Connector( this, type, uid, startpin, endpin );
                    con->setPointList( pointList );
                    conList.append( con );
                    startpin->isMoved();
                    endpin->isMoved();
                }
                else if( !m_pasting )// Start or End pin not found
                {
                    if( !startpin )
                        qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null startpin in Connector" << uid << startpinid;
                    if( !endpin )
                        qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null endpin in Connector"   << uid << endpinid;
            }   }
            else if( type == "Node")
            {
                Node* joint = new Node( this, type, newUid );
                if( m_pasting )
                {
                    m_idMap[uid] = newUid; // Map simu id to new id
                    joint->setSelected( true );
                }
                QString name = "";
                for( QStringRef prop : properties )
                {
                    if( name.isEmpty() ) { name = prop.toString(); continue; }
                    if     ( name == "Pos") joint->setPropStr( "Pos", prop.toString() );
                    else if( name == "x"  ) joint->setX( prop.toInt() );
                    else if( name == "y"  ) joint->setY( prop.toInt() );
                    name = "";
                }
                int number = joint->objectName().split("-").last().toInt();
                if( number > m_seqNumber ) m_seqNumber = number;               // Adjust item counter: m_seqNumber
                addItem( joint );
                nodeList.append( joint );
            }
            else if( type == "Plotter" ) ;// Old Plotter widget;
            else if( type == "SerialPort");
                 //&& ( element.hasAttribute( "visible" ) )
                 //&& ( element.attribute( "visible" ) == "false" ) ) ;// Old Serial Port Widget
            else if( type == "SerialTerm") ; /// TODO
            else{
                bool oldArduino = false;
                if( type == "Arduino" ){
                    oldArduino = true;
                    type = "Subcircuit";
                    newUid = newUid.remove( "Arduino " );
                }
                else if( type == "AVR" ){
                    type = "MCU";
                    newUid = newUid.replace( "at", "" );
                }
                else if( type == "PIC" ){
                    type = "MCU";
                    newUid = newUid.replace( "pic", "p" );
                }
                else if( type == "Frequencimeter" ) type = "FreqMeter";
                lastComp = NULL;
                Component* comp = createItem( type, newUid );
                if( comp )
                {
                    lastComp = comp;
                    if( m_pasting ) m_idMap[uid] = newUid;

                    Mcu* mcu = NULL;
                    if( oldArduino )
                    {
                        SubCircuit* subci = static_cast<SubCircuit*>(comp);
                        mcu = static_cast<Mcu*>( subci->getMainComp() );
                    }
                    else if( comp->itemType() == "Subcircuit")
                    {
                        SubCircuit* shield = static_cast<SubCircuit*>(comp);
                        if( shield->subcType() == Chip::Shield ) shieldList.append( shield );
                    }
                    comp->setIdLabel( label );
                    QString propName = "";
                    for( QStringRef prop : properties )
                    {
                        if( propName.isEmpty() ) { propName = prop.toString(); continue; }
                        QString value = prop.toString();
                        if( !comp->setPropStr( propName, value ) ) // SUBSTITUTIONS
                        {
                            if( propName == "Propagation_Delay_ns") { propName = "Tpd_ps"; value.append("000"); }
                            else                                    Component::substitution( propName );

                            if( !comp->setPropStr( propName, value ) )
                            {
                                bool ok = false;
                                if( oldArduino && mcu ) ok = mcu->setPropStr( propName, value );
                                if( !ok ){
                                    if( propName.toLower()  != "tristate" )   // TODELETE
                                    qDebug() << "Circuit: Wrong Property: "<<type<<newUid<<propName<<value;}
                            }
                        }
                        propName = "";
                    }
                    int number = comp->objectName().split("-").last().toInt();
                    if( number > m_seqNumber ) m_seqNumber = number;               // Adjust item counter: m_seqNumber
                    addItem( comp );
                    comp->updtLabelPos();
                    comp->updtValLabelPos();
                    compList.append( comp );
                }
                else qDebug() << " ERROR Creating Component: "<< type << uid;
    }   }   }
    if( m_pasting )
    {
        for( Component* item : compList ) { item->setSelected( true ); item->move( m_deltaMove ); }
        for( Component* item : nodeList ) { item->setSelected( true ); item->move( m_deltaMove ); }
        for( Connector* con : conList )   { con->setSelected( true );  con->move( m_deltaMove ); }
    }
    m_compList.append( compList );
    m_nodeList.append( nodeList );
    m_conList.append( conList );
    // Take care about unconnected Joints
    for( Node* joint : nodeList ) joint->remove(); // Only removed if some missing connector
    for( SubCircuit* shield : shieldList ) shield->connectBoard();

    m_idMap.clear();
    /// m_pinMap.clear();
    m_busy = false;
    QApplication::restoreOverrideCursor();
}

QString Circuit::circuitHeader()
{
    QString header = "<circuit type=\""+m_circType+"\" ";
    header += "stepSize=\""+QString::number( m_simulator->stepSize() )+"\" ";
    header += "stepsPS=\""+QString::number( m_simulator->stepsPerSec() )+"\" ";
    header += "NLsteps=\""+QString::number( m_simulator->maxNlSteps() )+"\" ";
    header += "animate=\""+QString::number( animate() )+"\" >\n";
    return header;
}
QString Circuit::circuitToString()
{
    QString circuit = circuitHeader();
    for( Component* comp : m_compList ) circuit += comp->toString();
    for( Node* node      : m_nodeList ) circuit += node->toString();
    for( Connector* conn : m_conList )  circuit += conn->toString();
    circuit += "\n</circuit>";
    return circuit;
}

bool Circuit::saveString( QString &fileName, QString doc )
{
    QFile file( fileName );
    if( !file.open( QFile::WriteOnly | QFile::Text ))
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning( NULL, "Circuit::saveString",
        tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc;
    file.close();

    return true;
}

bool Circuit::saveCircuit( QString fileName )
{
    if( m_conStarted ) return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString oldFilePath = m_filePath;
    m_filePath = fileName;

    bool saved = saveString( fileName, circuitToString() );
    if( saved )
    {
        qDebug() << "\nCircuit Saved: \n" << fileName;
        QFile file( m_backupPath );
        if( file.exists() ) QFile::remove( m_backupPath ); // Remove backup file
    }
    else m_filePath = oldFilePath;

    CompBase::m_saveBoard  = false;
    QApplication::restoreOverrideCursor();
    return saved;
}

void Circuit::importCirc(  QPointF eventpoint  )
{
    if( m_conStarted ) return;
    m_pasting = true;

    m_deltaMove = QPointF( 160, 160 );//togrid(eventpoint);

    const QString dir = m_filePath;
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Circuit"), dir,
                                          tr("Circuits (*.sim*);;All files (*.*)"));

    if( !fileName.isEmpty()
     && (fileName.endsWith(".simu") || fileName.endsWith(".sim1")) )
        loadCircuit( fileName );

    m_pasting = false;
}

Component* Circuit::createItem( QString type, QString id )
{
    Component* comp = NULL;
    for( LibraryItem* libItem : ItemLibrary::self()->items() )
    {
        if( libItem->type() != type ) continue;

        comp = libItem->createItemFnPtr()( this, type, id );
        if( comp )
        {
            QString category = libItem->category();
            if( ( category != "Meters" )
            &&  ( category != "Sources" )
            &&  ( category != "Other" ) )
                comp->m_printable = true;
    }   }
    return comp;
}

void Circuit::removeItems()                     // Remove Selected items
{
    if( m_conStarted ) return;
    saveState();
    m_busy = true;

    QList<Connector*> connectors;
    QList<Component*> components;

    for( QGraphicsItem* item : selectedItems() )
    {
        if( item->type() == QGraphicsItem::UserType+1 ) // Component
        {
            Component* comp = qgraphicsitem_cast<Component* >( item );
            if( m_compList.contains( comp ) && !components.contains( comp ) )
                components.append( comp );
        }
        else if( item->type() == QGraphicsItem::UserType+2 ) // ConnectorLine
        {
            ConnectorLine* line = qgraphicsitem_cast<ConnectorLine* >( item );
            Connector* con = line->connector();
            if( !connectors.contains( con ) ) connectors.append( con );
    }   }
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    for( Connector* con : connectors ) con->remove();
    for( Component* comp : components ) removeComp( comp );
    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
    m_busy = false;
}

void Circuit::removeComp( Component* comp )
{
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_compRemoved = false;
    comp->remove();
    if( !m_compRemoved ) return;

    if( m_compList.contains( comp ) )
    {
        if( comp->scene() ) removeItem( comp );
        m_compList.removeOne( comp );
    }
    delete comp; // crash in recent versions bcos already removed in removeItem( comp );
}

void Circuit::remove() // Remove everything ( Clear Circuit )
{
    if( m_conStarted ) return;
    m_busy = true;
    while( !m_compList.isEmpty() ) removeComp( m_compList.takeFirst() );
    m_busy = false;
}

void Circuit::deselectAll()
{ for( QGraphicsItem* item : selectedItems() ) item->setSelected( false ); }

void Circuit::saveState()
{
    if( m_conStarted ) return;
    m_changed = true;

    if( m_busy || m_deleting ) return;
    m_deleting = true;

    m_redoStack.clear();
    m_undoStack.append(  circuitToString() );

    QString title = MainWindow::self()->windowTitle();
    if( !title.endsWith('*') ) MainWindow::self()->setWindowTitle(title+'*');

    m_deleting = false;
}

void Circuit::saveChanges()
{
    if( m_simulator->isRunning() ) return;
    if( !m_changed || m_conStarted || m_busy || m_deleting ) return;
    m_changed = false;
    m_busy = true;

    saveString( m_backupPath, circuitToString() ); // Backup file
    m_busy = false;
}

void Circuit::undo()
{
    if( m_busy || m_deleting || m_conStarted || m_undoStack.isEmpty() ) return;
    m_busy = true;

    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_redoStack.prepend( circuitToString() );
    remove();
    m_seqNumber = 0;

    QString circuit = m_undoStack.takeLast();
    loadStrDoc( circuit );
    m_busy = false;
}

void Circuit::redo()
{
    if( m_busy || m_deleting || m_conStarted || m_redoStack.isEmpty() ) return;
    m_busy = true;

    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_undoStack.append( circuitToString() );
    remove();
    m_seqNumber = 0;

    QString circuit = m_redoStack.takeFirst();
    loadStrDoc( circuit );
    m_busy = false;
}

void Circuit::copy( QPointF eventpoint )
{
    if( m_conStarted ) return;

    m_eventpoint = togrid(eventpoint);

    QList<CompBase*> complist;
    QList<CompBase*> conlist;
    QList<QGraphicsItem*> itemlist = selectedItems();

    for( QGraphicsItem* item : itemlist )
    {
        if( item->type() == QGraphicsItem::UserType+1 ) // Component
        {
            Component* comp =  qgraphicsitem_cast<Component*>( item );
            if( comp ) complist.append( comp );
        }
        else if( item->type() == QGraphicsItem::UserType+2 ) // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( item );
            Connector* con = line->connector();
            if( !conlist.contains( con ) ) conlist.append( con );
    }   }
    QString circuit;
    for( CompBase* comp : complist ) circuit += comp->toString();
    for( CompBase* con :  conlist )  circuit += con->toString();

    QApplication::clipboard()->setText( circuit );
}

void Circuit::paste( QPointF eventpoint )
{
    if( m_conStarted ) return;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    
    bool animate = m_animate;
    saveState();
    m_busy = true;
    m_pasting = true;
    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    m_deltaMove = togrid(eventpoint) - m_eventpoint;

    QString circuit = circuitHeader();
    circuit += QApplication::clipboard()->text();
    circuit += "\n</circuit>";

    loadStrDoc( circuit );

    setAnimate( animate );
    m_pasting = false;
    m_busy = false;
}

void Circuit::newconnector( Pin* startpin )
{
    saveState();
    m_conStarted = true;

    QString type = QString("Connector");
    QString id = type;
    id.append( "-" );
    id.append( newSceneId() );

    new_connector = new Connector( this, type, id, startpin );
    m_conList.append( new_connector );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();
    new_connector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );
}

void Circuit::closeconnector( Pin* endpin )
{
    m_conStarted = false;
    new_connector->closeCon( endpin, /*connect=*/true );
}

void Circuit::deleteNewConnector()
{
    if( !m_conStarted ) return;
    new_connector->remove();
    m_conStarted = false;
}

void Circuit::updateConnectors()
{ for( Connector* con : m_conList ) con->updateLines(); }

void Circuit::addNode( Node* node )
{
    addItem( node );
    m_nodeList.append( node );
}

void Circuit::bom()
{
    if( m_conStarted ) return;

    QString fileName = QFileDialog::getSaveFileName( MainWindow::self()
                            , tr( "Bill Of Materials" )
                            , changeExt( m_filePath, "-bom.txt" )
                            , "(*.*)"  );
    if( fileName.isEmpty() ) return;

    QStringList bom;
    for( Component* comp : m_compList ) bom.append( comp->print() );

    QFile file( fileName );
    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "Circuit::bom",
          tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    }
    bom.sort();

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out <<  "\nCircuit: ";
    out <<  QFileInfo( m_filePath ).fileName();
    out <<  "\n\n";
    out <<  "Bill of Materials:\n\n";
    for( QString line : bom ) out << line;

    file.close();
}

void Circuit::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        if( m_conStarted )  event->accept();
        QGraphicsScene::mousePressEvent( event );
    }
    else if( event->button() == Qt::RightButton )
    {
        if( m_conStarted ) event->accept();
        else                QGraphicsScene::mousePressEvent( event );
    }
    else if( event->button() == Qt::MidButton )
    {
        QGraphicsScene::mousePressEvent( event );
}   }

void Circuit::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        if( m_conStarted )  new_connector->incActLine() ;
        QGraphicsScene::mouseReleaseEvent( event );
    }
    else if( event->button() == Qt::RightButton )
    {
        if( !m_conStarted ) QGraphicsScene::mouseReleaseEvent( event );
}   }

void Circuit::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_conStarted )
    {
        event->accept();
        if(event->modifiers() & Qt::ShiftModifier) new_connector->m_freeLine = true;
        new_connector->updateConRoute( 0l, event->scenePos() );
    }
    QGraphicsScene::mouseMoveEvent( event );
}

void Circuit::keyPressEvent( QKeyEvent* event )
{
    int key = event->key();

    if( m_conStarted )
    {
        if( key == Qt::Key_Escape ) deleteNewConnector();
        else QGraphicsScene::keyPressEvent( event );
        return;
    }
    if( event->modifiers() & Qt::ControlModifier )
    {
        if( key == Qt::Key_C )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            copy( m_graphicView->mapToScene( p ) );
            clearSelection();
        }
        if( key == Qt::Key_X )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            copy( m_graphicView->mapToScene( p ) );
            removeItems();
        }
        else if( key == Qt::Key_V )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            paste( m_graphicView->mapToScene( p ) );
        }
        else if( key == Qt::Key_Z )
        {
            if( event->modifiers() & Qt::ShiftModifier) redo();
            else undo();
        }
        else if( key == Qt::Key_Y ) redo();
        else if( key == Qt::Key_N ) CircuitWidget::self()->newCircuit();
        else if( key == Qt::Key_S )
        {
            if( event->modifiers() & Qt::ShiftModifier)
                 CircuitWidget::self()->saveCircAs();
            else CircuitWidget::self()->saveCirc();
        }
        else if( key == Qt::Key_O ) CircuitWidget::self()->openCirc();
        else if( key == Qt::Key_A )
        {
            for( Component* com : m_compList ) com->setSelected( true );
            for( Node*      nod : m_nodeList ) nod->setSelected( true );
            for( Connector* con : m_conList  ) con->setSelected( true );
        }
        else QGraphicsScene::keyPressEvent( event );
    }
    else if( key == Qt::Key_Delete || key == Qt::Key_Backspace )
    {
        removeItems();
        QGraphicsScene::keyPressEvent( event );
    } else {
        if( !event->isAutoRepeat() ) // Deliver Key events ( switches )
        {
            QString keys = event->text();
            while( keys.size() > 0 )
            {
                QString key = keys.left( 1 );
                keys.remove( 0, 1 );
                emit keyEvent( key, true );
        }  }
        QGraphicsScene::keyPressEvent( event );
}   }

void Circuit::keyReleaseEvent( QKeyEvent* event )
{
    if( !event->isAutoRepeat()
     && !( event->modifiers() & Qt::ControlModifier ) )  // Deliver Key events ( switches )
    {
        QString keys = event->text();
        while( keys.size() > 0 )
        {
            QString key = keys.left( 1 );
            keys.remove( 0, 1 );
            emit keyEvent( key, false );
    }   }
    QGraphicsScene::keyReleaseEvent( event );
}

void Circuit::dropEvent( QGraphicsSceneDragDropEvent* event )
{
    QString id   = event->mimeData()->text();
    QString file = "file://";

    if( id.startsWith( file ) )
    {
        id.replace( file, "" ).replace("\r\n", "" ).replace("%20", " ");
#ifdef _WIN32
        if( id.startsWith( "/" )) id.remove( 0, 1 );
#endif
        QString loId = id.toLower();
        if( loId.endsWith( ".jpg") || loId.endsWith( ".png") || loId.endsWith( ".gif"))
        {
            file = id;
            id   = "Image";
            Component* enterItem = createItem( "Image", newSceneId() );
            if( enterItem )
            {
                saveState();
                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( CircuitView::self()->mapToScene( cPos ) );
                enterItem->setBackground( file );
                addItem( enterItem );
                m_compList.append( enterItem );
        }   }
        else CircuitWidget::self()->loadCirc( id );
}   }

void Circuit::drawBackground( QPainter*  painter, const QRectF &rect )
{
    /*painter->setBrush(QColor( 255, 255, 255 ) );
    painter->drawRect( m_scenerect );
    return;*/

    painter->setBrush( QColor( 240, 240, 210 ) );
    painter->drawRect( m_scenerect );
    painter->setPen( QColor( 210, 210, 210 ) );

    if( m_hideGrid ) return;

    int startx = int(m_scenerect.x());///2;
    int endx   = int(m_scenerect.width())/2;
    int starty = int(m_scenerect.y());///2;
    int endy   = int(m_scenerect.height())/2;

    for( int i=4; i<endx; i+=8 ){
        painter->drawLine( i, starty, i, endy );
        painter->drawLine(-i, starty,-i, endy );
    }
    for( int i=4; i<endy; i+=8 ){
        painter->drawLine( startx, i, endx, i);
        painter->drawLine( startx,-i, endx,-i);
}   }

void Circuit::updatePin( ePin* epin, QString oldId, QString newId )
{
    remPin( oldId );
    Pin* pin = static_cast<Pin*>( epin );
    addPin( pin, newId );
}

void Circuit::setDrawGrid( bool draw )
{
    m_hideGrid = !draw;
    if( m_hideGrid ) MainWindow::self()->settings()->setValue( "Circuit/hideGrid", "true" );
    else             MainWindow::self()->settings()->setValue( "Circuit/hideGrid", "false" );
    update();
}

void Circuit::setShowScroll( bool show )
{
    m_showScroll = show;
    if( show ){
        m_graphicView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        m_graphicView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "true" );
    }else{
        m_graphicView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        m_graphicView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "false" );
}   }

int Circuit::autoBck() { return MainWindow::self()->autoBck(); }
void Circuit::setAutoBck( int secs )
{
    m_bckpTimer.stop();
    if( secs < 1 ) secs = 0;
    else           m_bckpTimer.start( secs*1000 );

    MainWindow::self()->setAutoBck( secs );
}

#include "moc_circuit.cpp"
