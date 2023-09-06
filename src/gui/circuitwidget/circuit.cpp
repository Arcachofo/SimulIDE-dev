/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
#include "comproperty.h"
#include "connectorline.h"
#include "componentselector.h"
#include "node.h"
#include "utils.h"
#include "subcircuit.h"
#include "subpackage.h"
#include "mcu.h"
#include "simulator.h"
#include "e-node.h"
#include "shield.h"
#include "linkable.h"

Circuit* Circuit::m_pSelf = NULL;

Circuit::Circuit( qreal x, qreal y, qreal width, qreal height, CircuitView*  parent )
       : QGraphicsScene( x, y, width, height, parent )
{
    m_simulator = new Simulator();

    setObjectName( "Circuit" );
    setParent( parent );
    m_graphicView = parent;
    m_scenerect.setRect( x, y, width, height );
    setSceneRect( QRectF(x, y, width, height) );

    m_pSelf = this;

    m_busy       = false;
    m_undo       = false;
    m_redo       = false;
    m_changed    = false;
    m_animate    = false;
    m_pasting    = false;
    m_deleting   = false;
    m_loading    = false;
    m_conStarted = false;
    m_createSubc = false;
    m_acceptKeys = true;
    m_batchProcess = 0;

    m_board = NULL;
    m_newConnector = NULL;
    m_seqNumber = 0;
    m_conNumber = 0;
    m_maxUndoSteps = 100;
    m_undoIndex = -1;
    m_redoIndex = -1;

    m_backupPath = MainWindow::self()->getConfigPath("backup.sim1");
    m_hideGrid   = MainWindow::self()->settings()->value( "Circuit/hideGrid" ).toBool();
    m_showScroll = MainWindow::self()->settings()->value( "Circuit/showScroll" ).toBool();
    m_filePath   = "";//qApp->applicationDirPath()+"/new.simu"; // AppImage tries to write in read olny filesystem

    connect( &m_bckpTimer, &QTimer::timeout,
                     this,&Circuit::saveBackup, Qt::UniqueConnection );
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
    for( Component* comp : m_compList ) if( comp->getUid() == id ) return comp;
    return NULL;
}

QString Circuit::getSeqNumber( QString name )
{
    QStringList words = name.split("-");
    for( int i=1; i<words.size(); ++i )    // Start at second word, first must be name
    {
        QString word = words.at( i );
        bool ok;
        word.toInt( &ok );  // If it converts to int, then this is old seqNumber
        if( ok ) return word;
    }
    return "";
}

QString Circuit::replaceId( QString pinName )
{
    QStringList words = pinName.split("-");
    for( int i=1; i<words.size(); ++i )    // Start at second word, first must be name
    {
        QString word = words.at( i );
        bool ok;
        word.toInt( &ok );  // If it converts to int, then this is old seqNumber
        if( ok )
        {
            words.replace( i, m_idMap.value( word ) );
            break;
        }
    }
    return words.join("-");
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

    m_busy = true;
    m_loading = true;
    m_filePath = fileName;
    m_error = 0;

    QString doc = fileToString( fileName, "Circuit::loadCircuit" );
    loadStrDoc( doc );

    m_busy = false;
    m_loading = false;

    if( m_error != 0 ) clearCircuit();
    else{
        m_graphicView->zoomToFit();
        qDebug() << "Circuit Loaded: ";
        qDebug() << fileName;
}   }

void Circuit::loadStrDoc( QString &doc )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Linkable*> linkList;   // Linked  Component list
    QSet<Component*> compList;   // Pasting Component list
    QSet<Connector*> conList;    // Pasting Connector list
    QSet<Node*> nodeList;        // Pasting node list

    m_newComp = NULL;
    Component* lastComp = NULL;
    QList<ShieldSubc*> shieldList;

    int rev = 0;
    m_busy  = true;
    if( !m_undo && !m_redo ) m_LdPinMap.clear();

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

                if     ( name == "stepSize") m_simulator->setStepSize( val.toULongLong() );
                else if( name == "stepsPS" ) m_simulator->setStepsPerSec( val.toULongLong() );
                else if( name == "NLsteps" ) m_simulator->setMaxNlSteps( val.toUInt() );
                else if( name == "reaStep" ) m_simulator->setreactStep( val.toULongLong() );
                else if( name == "animate" ) setAnimate( val.toInt() );
                else if( name == "rev"     ) rev = val.toInt();
            }
        }
        else if( line.contains("<mainCompProps") )
        {
            if( !lastComp ) continue;
            SubCircuit* subci = static_cast<SubCircuit*>(lastComp);
            Component* mComp = subci->getMainComp();      // Old circuits with only 1 MainComp
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
                if( propName == "MainCompId")  // If more than 1 mainComp then get Component
                {
                    QString compName = prop.toString();
                    mComp = subci->getMainComp( compName );
                    if( !mComp ) qDebug() << "ERROR: Could not create Main Component:"<< compName;
                }
                else if( mComp ) mComp->setPropStr( propName, prop.toString() );

                propName = "";
            }
        }
        else if( line.startsWith("<item") )
        {
            QString uid, newUid, type, label, newNum;

            QStringRef name;
            QVector<QStringRef> props = line.split("\"");
            QHash<QStringRef, QStringRef> properties;
            for( QStringRef prop : props )
            {
                if( prop.size() > 1 && prop.endsWith("=") )
                {
                    prop = prop.split(" ").last();
                    name = prop.mid( 0, prop.length()-1 );
                    continue;
                }
                else if( prop.endsWith(">") ) continue;
                else{
                    if     ( name == "itemtype"   ) type  = prop.toString();
                    else if( name == "uid"        ) uid   = prop.toString();
                    else if( name == "CircId"     ) uid   = prop.toString();
                    else if( name == "objectName" ) uid   = prop.toString();
                    else if( name == "label"      ) label = prop.toString();
                    else if( name == "id"         ) label = prop.toString();
                    else properties[name] = prop ;
            }   }
            if( type.isEmpty() ) { qDebug() << "ERROR: Component with no type:"<<label<< uid; continue;}

            if( m_pasting ) // Create new id
            {
                if( type == "Connector" ) newUid = "Connector-"+newConnectorId();
                else{
                    newNum = newSceneId();
                    if( type == "Subcircuit" || type == "MCU" )
                        newUid = uid.split("-").first()+"-"+newNum;
                    else newUid = type+"-"+newNum;
                }
            }
            else newUid = uid;

            if( type == "Connector" )
            {
                Pin* startpin = NULL;
                Pin* endpin   = NULL;
                QString startpinid, endpinid;
                QStringList pointList;

                QString name;
                QStringRef val;
                for( QStringRef prop : properties.keys() )
                {
                    name = prop.toString();
                    val  = properties.value( prop );

                    if     ( name == "startpinid") startpinid = val.toString();
                    else if( name == "endpinid"  ) endpinid   = val.toString();
                    else if( name == "pointList" ) pointList  = val.toString().split(",");
                    else if( name == "uid"       ) uid        = val.toString();
                }
                if( m_pasting )
                {
                    startpinid = replaceId( startpinid );
                    endpinid   = replaceId( endpinid );
                }
                if( m_undo || m_redo )
                {
                    startpin = m_pinMap.value( startpinid );
                    endpin   = m_pinMap.value( endpinid );
                }else{
                    startpin = m_LdPinMap.value( startpinid );
                    endpin   = m_LdPinMap.value( endpinid );
                }

                if( !startpin && !m_pasting ) // Pin not found by name... find it by pos
                {
                    int itemX = pointList.first().toInt();
                    int itemY = pointList.at(1).toInt();
                    startpin = findPin( itemX, itemY, startpinid );
                }
                if( !endpin && !m_pasting ) // Pin not found by name... find it by pos
                {
                    int itemX = pointList.at( pointList.size()-2 ).toInt();
                    int itemY = pointList.last().toInt();
                    endpin = findPin( itemX, itemY, endpinid );
                }
                if( startpin && startpin->connector() ) startpin = NULL;
                if( endpin   && endpin->connector()   ) endpin   = NULL;


                if( startpin && endpin )    // Create Connector
                {
                    if( newUid.isEmpty() ) newUid = "connector-"+newConnectorId();
                    Connector* con = new Connector( type, newUid, startpin, endpin );
                    con->setPointList( pointList );
                    conList.insert( con );
                    m_newComp = con;
                    if( !rev ){
                        startpin->isMoved();
                        endpin->isMoved();
                    }
                    int number = newUid.split("-").last().toInt();
                    if( number > m_conNumber ) m_conNumber = number; // Adjust Connector counter: m_conNumber
                }
                else if( !m_pasting /*&& !m_undo && !m_redo*/ )// Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null startpin in Connector" << uid << startpinid;
                    if( !endpin   ) qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null endpin in Connector"   << uid << endpinid;
            }   }
            else if( type == "Node")
            {
                Node* joint = new Node( type, newUid );
                if( m_pasting )
                {
                    m_idMap[getSeqNumber( uid )] = newNum; // Map simu id to new id
                    joint->setSelected( true );
                }
                QString name;
                QStringRef val;
                for( QStringRef prop : properties.keys() )
                {
                    name = prop.toString();
                    val  = properties.value( prop );
                    if     ( name == "Pos") joint->setPropStr( "Pos", val.toString() );
                    else if( name == "x"  ) joint->setX( val.toInt() );
                    else if( name == "y"  ) joint->setY( val.toInt() );
                }
                int number = joint->getUid().split("-").last().toInt();
                if( number > m_seqNumber ) m_seqNumber = number; // Adjust item counter: m_seqNumber
                addItem( joint );
                nodeList.insert( joint );
                m_newComp = joint;
                m_compMap[newUid] = joint;
            }
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
                    m_newComp = comp;
                    lastComp = comp;
                    if( m_pasting ) m_idMap[getSeqNumber( uid )] = newNum; // Map simu id to new id

                    Mcu* mcu = NULL;
                    if( oldArduino )
                    {
                        SubCircuit* subci = static_cast<SubCircuit*>(comp);
                        mcu = static_cast<Mcu*>( subci->getMainComp() );
                    }
                    if( comp->itemType() == "Subcircuit")
                    {
                        ShieldSubc* shield = static_cast<ShieldSubc*>(comp);
                        if( shield->subcType() >= Chip::Shield ) shieldList.append( shield );
                    }
                    comp->setPropStr( "label", label ); //setIdLabel( label );

                    QList<propGroup>* groups = comp->properties(); // Set properties in correct order
                    for( propGroup group : *groups )
                    {
                        QList<ComProperty*> propList = group.propList;
                        if( propList.isEmpty() ) continue;
                        for( ComProperty* prop : propList )
                        {
                            QString qpn = prop->name();
                            QStringRef pn( &qpn );
                            if( !properties.contains( pn ) ) continue;
                            prop->setValStr( properties.value( pn ).toString() );
                            properties.remove( pn );
                        }
                    }
                    QString propName;
                    QString value;
                    for( QStringRef prop : properties.keys() ) // Properties not recognized (old versions)
                    {
                        propName = prop.toString();
                        value    = properties.value( prop ).toString();
                        if( !comp->setPropStr( propName, value ) ) // SUBSTITUTIONS
                        {
                            if( propName == "Propagation_Delay_ns") { propName = "Tpd_ps"; value.append("000"); }
                            else                                    Component::substitution( propName );

                            if( !comp->setPropStr( propName, value ) )
                                if( oldArduino && mcu ) mcu->setPropStr( propName, value );
                        }
                    }
                    int number = comp->getUid().split("-").last().toInt();
                    if( number > m_seqNumber ) m_seqNumber = number;               // Adjust item counter: m_seqNumber
                    addItem( comp );
                    if( m_pasting ) comp->setIdLabel( newUid );
                    comp->updtLabelPos();
                    comp->updtValLabelPos();
                    compList.insert( comp );
                    if( comp->m_linkable )
                    {
                        Linkable* l = dynamic_cast<Linkable*>(comp);
                        if( l->hasLinks() ) linkList.append( l );
                    }
                }
                else qDebug() << " ERROR Creating Component: "<< type << uid;
    }   }   }
    if( m_pasting )
    {
        for( Component* comp : compList ) { comp->setSelected( true ); comp->move( m_deltaMove ); }
        for( Component* comp : nodeList ) { comp->setSelected( true ); comp->move( m_deltaMove ); }
        for( Connector* con  : conList )  { con->setSelected( true );  con->move( m_deltaMove ); }
    }
    else for( Component* comp : compList ) { comp->moveSignal(); }

    for( Linkable* l : linkList )
        l->createLinks( &compList );

    m_compList.unite( compList );
    m_nodeList.unite( nodeList );
    m_connList.unite( conList );

    // Take care about unconnected Joints
    if( !m_undo && !m_redo )  for( Node* joint : nodeList ) joint->checkRemove(); // Only removed if some missing connector
    for( ShieldSubc* shield : shieldList ) shield->connectBoard();

    setAnimate( m_animate ); // Force Pin update

    if( m_pasting ) m_idMap.clear();
    m_busy = false;
    QApplication::restoreOverrideCursor();
    update();
}

QString Circuit::circuitHeader()
{
    QString header = "<circuit version=\""+QString( APP_VERSION )+"\" rev=\""+QString( REVNO )+"\" ";
    header += "stepSize=\""+QString::number( m_simulator->stepSize() )+"\" ";
    header += "stepsPS=\""+QString::number( m_simulator->stepsPerSec() )+"\" ";
    header += "NLsteps=\""+QString::number( m_simulator->maxNlSteps() )+"\" ";
    header += "reaStep=\""+QString::number( m_simulator->reactStep() )+"\" ";
    header += "animate=\""+QString::number( m_animate )+"\" >\n";
    return header;
}
QString Circuit::circuitToString()
{
    if( m_board && m_board->m_boardMode ) m_board->setBoardMode( false );

    QString circuit = circuitHeader();
    for( Component* comp : m_compList ) circuit += comp->toString();
    for( Node* node      : m_nodeList ) circuit += node->toString();
    for( Connector* conn : m_connList )  circuit += conn->toString();
    circuit += "\n</circuit>";

    if( m_board && m_board->m_boardMode ) m_board->setBoardMode( true );
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

    QApplication::restoreOverrideCursor();
    return saved;
}

void Circuit::importCirc(  QPointF eventpoint  )
{
    if( m_conStarted ) return;
    m_pasting = true;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_deltaMove = QPointF( 160, 160 );//toGrid(eventpoint);

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
    for( LibraryItem* item : ItemLibrary::self()->items() )
    {
        if( !item->createItemFnPtr() ) continue; // Is category
        if( item->type() != type ) continue;

        comp = item->createItemFnPtr()( type, id );
    }
    m_compMap[id] = comp;
    return comp;
}

void Circuit::removeItems()                     // Remove Selected items
{
    if( m_conStarted ) return;
    m_busy = true;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    beginBatchProcess();
    QSet<Connector*> oldConns = m_connList;
    QSet<Component*> oldComps = m_compList;
    QSet<Node*>      oldNodes = m_nodeList;

    //save all comps
    QMap<CompBase*, QString> compStrMap;

    for( Connector* conn : oldConns ) compStrMap.insert( conn, conn->toString() );
    for( Node*      node : oldNodes ) compStrMap.insert( node, node->toString() );
    for( Component* comp : oldComps ) compStrMap.insert( comp, comp->toString() );

    QList<Connector*> conns;
    QList<Component*> comps;

    //remove all connectors
    for( QGraphicsItem* item : selectedItems() )
    {
        if( item->type() == QGraphicsItem::UserType+2 ) // ConnectorLine
        {
            ConnectorLine* line = qgraphicsitem_cast<ConnectorLine*>( item );
            Connector* con = line->connector();
            if( !conns.contains( con ) ) conns.append( con );
        }
        else if( item->type() == QGraphicsItem::UserType+1 ) // Component
        {
            Component* comp = qgraphicsitem_cast<Component*>( item );
            comps.append( comp );
        }
    }
    for( Connector* conn : conns ) removeConnector( conn );

    for( Component* comp: comps )
    {
        if( comp->itemType() == "Node" ) removeNode( (Node*)comp );
        else                             removeComp( comp );
    }
    /// addCompState( NULL, "New State", stateNew );


    for( Connector* conn : oldConns ) {
        if( !m_connList.contains( conn ) ) addCompRemovedState( conn->getUid(), compStrMap.value(conn) );
    }
    for( Node* node : oldNodes ) {
        if( !m_nodeList.contains( node ) ) addCompRemovedState( node->getUid(), compStrMap.value(node) );
    }
    for( Component* comp : oldComps ) {
        if( !m_compList.contains( comp ) ) addCompRemovedState( comp->getUid(), compStrMap.value(comp) );
    }
    for( Connector* conn : m_connList ) { // New Connectors
        if( !oldConns.contains( conn ) ) addCompState( conn, COMP_STATE_CREATED, stateAdd );
    }

    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
    saveState();
    m_busy = false;
}

void Circuit::removeComp( Component* comp )
{
    m_compRemoved = false;
    comp->remove();
    if( !m_compRemoved ) return;

    if( m_compList.contains( comp ) ) m_compList.remove( comp );
    if( comp->scene() ) removeItem( comp );
    m_compMap.remove( comp->getUid() );
    m_removedComps.insert( comp );
}

void Circuit::removeNode( Node* node )
{
    if( !m_nodeList.contains(node) ) return;
    if( m_deleting ) return;
    m_nodeList.remove( node );
    m_compMap.remove( node->getUid() );
    removeItem( node );
    m_removedComps.insert( node );
}

void Circuit::removeConnector( Connector* conn )
{
    if( !m_connList.contains(conn) ) return;
    conn->remove();
    m_connList.remove( conn );
    m_compMap.remove( conn->getUid() );
    m_removedComps.insert( conn );
}

/*void Circuit::clearRemovedComps()
{
    //for( Connector* conn : m_removedConns ) delete conn;
    for( CompBase* comp : m_removedComps ) delete comp;
    m_removedComps.clear();
    //m_removedConns.clear();
}*/

void Circuit::clearCircuit() // Remove everything ( Clear Circuit )
{
    if( m_conStarted ) return;
    m_deleting = true;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    QSet<Component*> compList = m_compList;
    for( Component* comp : compList )
    {
        comp->remove();
        if( comp->scene() ) removeItem( comp );
        delete comp;
    }
    QSet<Node*> nodeList = m_nodeList;
    for( Node* node : nodeList )
    {
        if( node->scene() ) removeItem( node );
        delete node;
    }
    m_deleting = false;
}

void Circuit::saveBackup()
{
    if( m_simulator->isRunning() ) return;
    if( !m_changed || m_conStarted || m_busy || m_deleting ) return;
    m_changed = false;

    m_busy = true;
    saveString( m_backupPath, circuitToString() ); // Backup file
    m_busy = false;
}

void Circuit::setChanged()
{
    m_changed = true;
    QString title = MainWindow::self()->windowTitle();
    if( !title.endsWith('*') ) MainWindow::self()->setWindowTitle(title+'*');
}

void Circuit::saveState()
{
    if( m_conStarted || m_circState.size() == 0 ) return;
    setChanged();

    if( m_undo )
    {
        m_redoStack.append( m_circState );
        m_redoIndex++;
    }
    else{                     // Redo or new state
        if( !m_redo ){
            if( m_redoStack.size() )
            {
                m_redoStack.clear();
                m_redoIndex = -1;
                while( m_undoStack.size() > (m_undoIndex+1) ) m_undoStack.removeLast();
            }
            m_undoStack.append( m_circState );
            if( m_undoStack.size() > m_maxUndoSteps )
            {
                m_undoStack.takeFirst();
                m_undoIndex--;
            }
        }
        m_undoIndex++;
    }
    m_circState.clear();
    m_batchProcess = 0;  // Ends all batchProcess

    //clearRemovedComps();
    for( CompBase* comp : m_removedComps ) delete comp;
    m_removedComps.clear();

qDebug() << "Circuit::saveState Undo:"<< m_undoStack.size()<<m_undoIndex
                              <<"Redo:"<<m_redoStack.size()<<m_redoIndex<<"\n";
}

void Circuit::unSaveState()
{
    if( m_undoStack.isEmpty() ) return;
    m_undoStack.takeLast();
    m_undoIndex--;
}

void Circuit::beginBatchProcess()
{
    qDebug() << "Circuit::beginBatchProcess";
    if( !m_batchProcess ) addCompState( NULL, "New State", stateNew );
    m_batchProcess++;
}

void Circuit::endBatchProcess()
{
    qDebug() << "Circuit::endBatchProcess";
    if( m_batchProcess > 0 ){
        m_batchProcess--;
        if( m_batchProcess == 0 ) saveState();
    }
}

void Circuit::addCompState( CompBase* c, QString p, stateMode state )
{
    if( m_loading || m_deleting ) return;
    QString type = "NULL";
    if( c ) type = c->itemType();

    if ( c ) qDebug() << "Circuit::addCompState    " << c->getUid() << p << state;
    else     qDebug() << "Circuit::addCompState    " << type<<p<<state;

    if( state & stateNew )
    {
        if( !m_batchProcess ) m_circState.clear(); // Don't clear state if in BatchProcess
    }
    if( state & stateAdd )
    {
        compState cState;
        Q_ASSERT( c );
        cState.component = c->getUid();
        cState.property  = p;
        if( p == COMP_STATE_REMOVED ) cState.valStr = c->toString();
        else                          cState.valStr = c->getPropStr( p );

        if( c->itemType() == "Connector" )
        {
            Connector* con = (Connector*)c;
            if( p == COMP_STATE_CREATED )
            {
                cState.valStr = con->startPinId(); // Save Connector start Pin id.
            }
        }
        if ( m_undo ) m_circState.compStates.prepend( cState );
        else m_circState.compStates.append( cState );
    }
    if( state & stateSave )
    {
        if( !m_batchProcess ) saveState();
    }
}

void Circuit::addCompRemovedState( const QString &id, const QString &strVal )
{
    qDebug() << "Circuit::addCompRemovedState" << id << COMP_STATE_REMOVED;
    compState cState;
    cState.component = id;
    cState.property  = COMP_STATE_REMOVED;
    cState.valStr    = strVal;

    if ( m_undo ) m_circState.compStates.prepend( cState );
    else          m_circState.compStates.append( cState );
}

bool Circuit::restoreState( circState step )
{
    beginBatchProcess();
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_busy = true;

    /// addCompState( NULL, "", stateNew ); // Start new Undo/Redo state

    int iStep, i;
    if( m_undo ) { iStep = -1; i = step.compStates.size() - 1 ; }
    else         { iStep =  1; i = 0; }

    while( i>=0 && i < step.compStates.size() )
    {
        compState cState = step.compStates[i];
        i += iStep;
        QString compName = cState.component;
        QString propName = cState.property;
        QString propVal  = cState.valStr;
        CompBase* comp   = m_compMap.value( compName );

qDebug() << "Circuit::restoreState -->"<<compName<<cState.property<<comp;

        if( propName == COMP_STATE_REMOVED )
        {
            loadStrDoc( propVal );
            if( m_newComp ) addCompState( m_newComp, COMP_STATE_CREATED, stateAdd );
        }
        else if( propName == COMP_STATE_CREATED )
        {
            if( comp && comp->itemType() != "Connector" )
            {
                addCompState( comp, COMP_STATE_REMOVED, stateAdd );

                if( comp->itemType() == "Node") removeNode( (Node*)comp );
                else                            removeComp( (Component*)comp );
            }
            else{
                QString propVal  = cState.valStr;
                Pin* pin = m_pinMap.value( propVal ); // Find Connector start Pin from id.
                if( !pin ) continue;

                Connector* con = pin->connector();
                if( !con ) continue;

                if( m_connList.contains( con ) ) {
                    addCompState( con, COMP_STATE_REMOVED, stateAdd );
                    removeConnector( con );
                }
            }
        } else // Property
        {
            if( !comp ){
                qDebug() << "Circuit::restoreState Error restoring"<<compName<<propName<<propVal;
                continue;
            }
            addCompState( comp, propName, stateAdd );
            comp->setPropStr( propName, propVal );
        }
    }
    m_LdPinMap.clear();
    m_busy = false;
    update();
    saveState();
    return true;
}

void Circuit::undo()
{
    if( m_busy || m_deleting || m_conStarted || m_undoIndex < 0 ) return;
qDebug() << "\nCircuit::undo"<<m_undoIndex<<m_redoIndex;

    if( m_undoIndex > m_undoStack.size() )
    {
        qDebug() << "Circuit::redo index Error"<< m_undoStack.size()<<m_undoIndex;
        clearUndoRedo();
        return;
    }
    m_undo = true;
    circState step = m_undoStack.at( m_undoIndex );
    if( restoreState( step ) )
    {
        m_undoIndex--;
        finishUndoRedo();
    }
    else clearUndoRedo();
    m_undo = false;

//qDebug() << "Circuit::undo Undo:"<<m_undoStack.size()<<m_undoIndex
//                        <<"Redo:"<<m_redoStack.size()<<m_redoIndex<<"\n";
//qDebug() << "Circuit::undo-----------------------\n";
}

void Circuit::redo()
{
    if( m_busy || m_deleting || m_conStarted || m_redoIndex < 0 ) return;
qDebug() << "\nCircuit::redo"<<m_undoIndex<<m_redoIndex;

    if( m_redoIndex > m_redoStack.size() )
    {
        qDebug() << "Circuit::redo index Error"<< m_redoStack.size()<<m_redoIndex;
        clearUndoRedo();
        return;
    }
    m_redo = true;
    circState step = m_redoStack.at( m_redoIndex );
    if( restoreState( step ) )
    {
        m_redoIndex--;
        finishUndoRedo();
        if( m_redoIndex < 0 ) m_redoStack.clear(); /// ???
    }
    else clearUndoRedo();
    m_redo = false;

//qDebug() << "Circuit::redo Undo:"<< m_undoStack.size()<<m_undoIndex
//                        <<"Redo:"<<m_redoStack.size()<<m_redoIndex<<"\n";
//qDebug() << "Circuit::redo-----------------------\n";
}

void Circuit::finishUndoRedo()
{
    for( Connector* con : m_connList ) {
        if( m_board && m_board->m_boardMode ) con->setVisib( false );
        else{
            con->startPin()->isMoved();
            con->endPin()->isMoved();
        }
    }
}

void Circuit::clearUndoRedo()
{
    qDebug() << "Circuit::clearUndoRedo Warning: Stack Cleared"<<m_redoIndex<<m_undoIndex<<"\n";
    m_redoStack.clear();
    m_undoStack.clear();
    m_redoIndex = -1;
    m_undoIndex = -1;
    m_busy = false;
}

void Circuit::copy( QPointF eventpoint )
{
    if( m_conStarted ) return;

    m_eventpoint = toGrid( eventpoint );

    QList<CompBase*> complist;
    QList<CompBase*> conlist;
    QList<QGraphicsItem*> itemlist = selectedItems();

    for( QGraphicsItem* item : itemlist )
    {
        if( item->type() == QGraphicsItem::UserType+1 ) // Component
        {
            Component* comp =  qgraphicsitem_cast<Component*>( item );
            if( comp && !comp->isHidden() && !comp->parentItem() ) complist.append( comp );
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
    m_busy    = true;
    m_pasting = true;
    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    m_deltaMove = toGrid(eventpoint) - m_eventpoint;

    QString circuit = circuitHeader();
    circuit += QApplication::clipboard()->text();
    circuit += "\n</circuit>";

    QSet<Connector*> conns = m_connList;
    QSet<Component*> comps = m_compList;
    QSet<Node*>      nodes = m_nodeList;
    beginBatchProcess();
    loadStrDoc( circuit );
    // save states for components created in loadStrDoc(): new_list - old_list
    for( Component* comp : m_compList-comps ) addCompState( comp, COMP_STATE_CREATED, stateAdd );
    for( Node*      node : m_nodeList-nodes ) addCompState( node, COMP_STATE_CREATED, stateAdd );
    for( Connector* conn : m_connList-conns ) addCompState( conn, COMP_STATE_CREATED, stateAdd );
    endBatchProcess();

    setAnimate( animate );
    m_pasting = false;
    m_busy = false;
}

void Circuit::newconnector( Pin* startpin )
{
    beginBatchProcess();
    /// addCompState( NULL, "", stateNew );
    m_conStarted = true;

    QString type = QString("Connector");
    QString id = type+"-"+newConnectorId() ;

    m_newConnector = new Connector( type, id, startpin );
    m_connList.insert( m_newConnector );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();
    m_newConnector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );
}

void Circuit::closeconnector( Pin* endpin )
{
    m_conStarted = false;
    m_newConnector->closeCon( endpin );
    addCompState( m_newConnector, COMP_STATE_CREATED, stateAdd );
    saveState();
}

void Circuit::deleteNewConnector()
{
    if( !m_conStarted ) return;
    removeConnector( m_newConnector );
    m_conStarted = false;
    unSaveState();
}

void Circuit::updateConnectors()
{ for( Connector* con : m_connList ) con->updateLines(); }

void Circuit::addNode( Node* node )
{
    addItem( node );
    m_nodeList.insert( node );
    m_compMap.insert( node->getUid(), node );
}

/*void Circuit::bom()
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
}*/

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
        QGraphicsScene::mousePressEvent( event );
}

void Circuit::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        if( m_conStarted )  m_newConnector->incActLine() ;
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
        if(event->modifiers() & Qt::ShiftModifier) m_newConnector->m_freeLine = true;
        m_newConnector->updateConRoute( 0l, event->scenePos() );
    }
    QGraphicsScene::mouseMoveEvent( event );
}

void Circuit::keyPressEvent( QKeyEvent* event )
{
    if( !m_acceptKeys )  // Text Component
    {
        QGraphicsScene::keyPressEvent( event );
        return;
    }

    int key = event->key();

    if( m_conStarted )
    {
        if( key == Qt::Key_Escape ) deleteNewConnector();
        else QGraphicsScene::keyPressEvent( event );
        return;
    }
    if( event->modifiers() & Qt::AltModifier ) // Create Component shortcut
    {
        QString str = event->text();
        if( str.isEmpty() )
        {
            QGraphicsScene::keyPressEvent( event );
            return;
        }
        QHash<QString, QString> shortCuts = ComponentSelector::self()->getShortCuts();
        QStringList keys = shortCuts.values();
        if( keys.contains( str ) )
        {
            QString compNam = shortCuts.key( str );

            Component* enterItem = createItem( compNam, compNam+"-"+newSceneId() );
            if( enterItem )
            {
                saveState();
                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( toGrid( CircuitView::self()->mapToScene( cPos ) ) );
                addItem( enterItem );
                m_compList.insert( enterItem );
            }
        }
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
            for( Connector* con : m_connList  ) con->setSelected( true );
        }
        else if( key == Qt::Key_R )
        {
            for( Component* com : m_compList ) if( com->isSelected() ) com->rotateCW();
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
                m_compList.insert( enterItem );
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

void Circuit::setAnimate( bool an )
{
    m_animate = an;
    for( Pin* pin : m_pinMap.values() ) pin->animate( an );
    update();
}

int Circuit::autoBck() { return MainWindow::self()->autoBck(); }
void Circuit::setAutoBck( int secs )
{
    m_bckpTimer.stop();
    if( secs < 1 ) secs = 0;
    else           m_bckpTimer.start( secs*1000 );

    MainWindow::self()->setAutoBck( secs );
}

#include "moc_circuit.cpp"
