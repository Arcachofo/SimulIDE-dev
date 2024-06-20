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
#include "componentlist.h"
#include "node.h"
#include "utils.h"
#include "subcircuit.h"
#include "subpackage.h"
#include "mcu.h"
#include "simulator.h"
#include "e-node.h"
#include "shield.h"
#include "linker.h"
#include "tunnel.h"
#include "createcomp.h"

Circuit* Circuit::m_pSelf = NULL;

Circuit::Circuit( qreal x, qreal y, qreal width, qreal height, CircuitView*  parent )
       : QGraphicsScene( x, y, width, height, parent )
{
    m_simulator = new Simulator();
    Tunnel::clearTunnels();

    setObjectName( "Circuit" );
    setParent( parent );
    m_graphicView = parent;
    m_scenerect.setRect( x, y, width, height );
    setSceneRect( QRectF(x, y, width, height) );

    m_pSelf = this;

    //m_converting = false;

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
    m_cicuitBatch = 0;

    m_creCompDialog = NULL;
    m_board = NULL;
    m_newConnector = NULL;
    m_seqNumber = 0;
    m_conNumber = 0;
    m_maxUndoSteps = 100;
    m_undoIndex = -1;

    m_backupPath = MainWindow::self()->getConfigPath("backup.sim1");
    m_hideGrid   = MainWindow::self()->settings()->value( "Circuit/hideGrid" ).toBool();
    m_filePath   = "";//qApp->applicationDirPath()+"/new.simu"; // AppImage tries to write in read olny filesystem

    connect( &m_bckpTimer, &QTimer::timeout,
                     this,&Circuit::saveBackup, Qt::UniqueConnection );

    qDebug() << endl << "-------------------------------------------------";
    qDebug() << "                   NEW CIRCUIT                   "<<endl;
}

Circuit::~Circuit()
{
    delete m_simulator;

    m_bckpTimer.stop();
    m_undoStack.clear();

    cancelComp();

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
        if( ok ){
            words.replace( i, m_idMap.value( word ) );
            break;
        }
    }
    return words.join("-");
}

Pin* Circuit::findPin( int x, int y, QString id )
{
    // qDebug() << "Circuit::findPin" << id;
    QRectF itemRect = QRectF ( x-4, y-4, 8, 8 );

    QList<QGraphicsItem*> list = items( itemRect ); // List of items in (x, y)
    for( QGraphicsItem* it : list )
    {
        Pin* pin =  qgraphicsitem_cast<Pin*>( it );
        if( pin && pin->pinId().left(1) == id.left(1) && !pin->connector() ) return pin; // Check if names start by same letter
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

    QList<Linker*> linkList;   // Linked  Component list
    QList<Component*> compList;   // Pasting Component list
    QList<Connector*> conList;    // Pasting Connector list
    QList<Node*> nodeList;        // Pasting node list

    m_newComp = NULL;
    Component* lastComp = NULL;
    QList<ShieldSubc*> shieldList;

    m_circRev = 0;
    m_busy  = true;
    if( !m_undo && !m_redo ) m_LdPinMap.clear();

    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.startsWith("<item") )
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
                    if     ( name == "itemtype"  ) type  = prop.toString();
                    else if( name == "uid"       ) uid   = prop.toString();
                    else if( name == "CircId"    ) uid   = prop.toString();
                    else if( name == "objectName") uid   = prop.toString();
                    else if( name == "label"     ) label = prop.toString();
                    else if( name == "id"        ) label = prop.toString();
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
                    //else if( name == "uid"       ) uid        = val.toString();
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
                    conList.append( con );
                    m_newComp = con;
                    if( m_circRev == 0 ){
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
                nodeList.append( joint );
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
                    if( type == "Package" ) compList.prepend( comp );
                    else                    compList.append( comp );
                    if( comp->m_isLinker ){
                        Linker* l = dynamic_cast<Linker*>(comp);
                        if( l->hasLinks() ) linkList.append( l );
                    }
                }
                else qDebug() << " ERROR Creating Component: "<< type << uid;
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
                    if( !mComp ) qDebug() << "ERROR: Could not get Main Component:"<< compName;
                }
                else if( mComp ) mComp->setPropStr( propName, prop.toString() );

                propName = "";
            }
        }
        else if( (line.startsWith("<circuit") || line.startsWith("<libitem") ) && !m_pasting )
        {
            line = line.mid( 9, line.length()-11 );
            QStringRef name;

            QVector<QStringRef> props = line.split("\"");
            QHash<QStringRef, QStringRef> properties;
            for( QStringRef prop : props )
            {
                //if( prop.size() < 2 ) continue;
                if( prop.endsWith("=") )
                {
                    prop = prop.split(" ").last();
                    name = prop.mid( 0, prop.length()-1 );
                    continue;
                }
                else if( prop.isEmpty() ) continue;
                else{
                    if     ( name == "stepSize") m_simulator->setStepSize( prop.toULongLong() );
                    else if( name == "stepsPS" ) m_simulator->setStepsPerSec(prop.toULongLong() );
                    else if( name == "NLsteps" ) m_simulator->setMaxNlSteps( prop.toUInt() );
                    else if( name == "reaStep" ) m_simulator->setreactStep( prop.toULongLong() );
                    else if( name == "animate" ) setAnimate( prop.toInt() );
                    else if( name == "rev"     ) m_circRev  = prop.toInt();
                    else if( name == "category") m_category = prop.toString();
                    else if( name == "compname") m_compName = prop.toString();
                    else if( name == "compinfo") m_compInfo = prop.toString();
                    else if( name == "icondata") m_iconData = prop.toString();
                    else if( name == "itemtype") m_itemType = prop.toString();
            }   }
        }
        else if( line.startsWith("</circuit") ) break;
    }
    if( m_pasting )
    {
        for( Component* comp : compList )
        {
            comp->setSelected( true ); comp->move( m_deltaMove );
            if( comp->itemType() == "Package" ) m_compList.prepend( comp );
            else                                m_compList.append( comp );
        }
        for( Node* nod : nodeList )
        {
            nod->setSelected( true );
            nod->move( m_deltaMove );
            m_nodeList.append( nod );

        }
        for( Connector* con  : conList  )
        {
            con->setSelected( true );
            con->move( m_deltaMove );
            m_connList.append( con );
        }
    }
    else{
        for( Component* comp : compList )
        {
            comp->moveSignal();
            m_compList.append( comp );
            //if( comp->itemType() == "Package" ) m_compList.prepend( comp );
            //else                                m_compList.append( comp );
        }
        m_nodeList += nodeList;
        m_connList += conList;
    }

    for( Linker* l : linkList ) l->createLinks( &compList );

    // Take care about unconnected Joints
    if( !m_undo && !m_redo )  for( Node* joint : nodeList ) joint->checkRemove(); // Only removed if some missing connector
    for( ShieldSubc* shield : shieldList ) shield->connectBoard();

    setAnimate( m_animate ); // Force Pin update

    if( m_pasting ) m_idMap.clear();

    if( !m_category.isEmpty() || !m_iconData.isEmpty() ) createComp(); // Add comp widget to bottom panel
    m_busy = false;
    QApplication::restoreOverrideCursor();
    update();
}

void Circuit::createComp()
{
    if( m_creCompDialog ) return;
    m_creCompDialog = new creCompDialog( CircuitWidget::self() );
    CircuitWidget::self()->panelSplitter()->addWidget( m_creCompDialog );
    m_creCompDialog->show();
}

void Circuit::cancelComp()
{
    if( m_creCompDialog ){
        m_creCompDialog->deleteLater();
        m_creCompDialog = nullptr;
    }
}

/*QString Circuit::circuitToComp( QString category, QString iconData, QString compType )
{
    m_category = category;
    m_iconData = iconData;
    m_compType = compType;
    QString component = circuitToString();

    if( !m_filePath.endsWith(".comp")){
        m_category = "";
        m_iconData = "";
        m_compType = "";
    }
    return component;
}*/

QString Circuit::circuitHeader()
{
    QString header = "<circuit version=\""+QString( APP_VERSION )+"\" ";
    header += "rev=\""     + QString::number( MainWindow::self()->revision() )+"\" ";
    header += "stepSize=\""+ QString::number( m_simulator->stepSize() )+"\" ";
    header += "stepsPS=\"" + QString::number( m_simulator->stepsPerSec() )+"\" ";
    header += "NLsteps=\"" + QString::number( m_simulator->maxNlSteps() )+"\" ";
    header += "reaStep=\"" + QString::number( m_simulator->reactStep() )+"\" ";
    header += "animate=\"" + QString::number( m_animate ? 1 : 0 )+"\" ";
    header += ">\n";
    return header;
}

QString Circuit::circuitToString()
{
    if( m_board && m_board->m_boardMode ) m_board->setBoardMode( false );

    QString circuit = circuitHeader();
    for( Component* comp : m_compList ) circuit += comp->toString();
    for( Node* node      : m_nodeList ) circuit += node->toString();
    for( Connector* conn : m_connList ) circuit += conn->toString();
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
    QTextStream out( &file );
    out.setCodec("UTF-8");
    out << doc;
    file.close();

    return true;
}

bool Circuit::saveCircuit( QString filePath )
{
    if( m_conStarted ) return false;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QString oldFilePath = m_filePath;
    m_filePath = filePath;

    bool saved = saveString( filePath, circuitToString() );
    if( saved )
    {
        qDebug() << "\nCircuit Saved: \n" << filePath;
        QFile file( m_backupPath );
        if( file.exists() ) QFile::remove( m_backupPath ); // Remove backup file
    }
    else m_filePath = oldFilePath;

    QApplication::restoreOverrideCursor();
    return saved;
}

void Circuit::importCircuit()
{
    if( m_conStarted ) return;

    m_deltaMove = QPointF( 0, 0 );

    QString filePath = QFileDialog::getOpenFileName( 0l, tr("Import Circuit"), m_filePath,
                                          tr("Circuits (*.sim*);;All files (*.*)"));

    if( !filePath.isEmpty()
     && ( filePath.endsWith(".simu") || filePath.endsWith(".sim1") ) )
    {
        QString doc = fileToString( filePath, "Circuit::importCirc" );
        QApplication::clipboard()->setText( doc );

        m_eventpoint = QPointF(0,0);
        paste( QPointF(0,0) );
    }
}

Component* Circuit::createItem( QString type, QString id, bool map )
{
    Component* comp = NULL;
    for( LibraryItem* item : ItemLibrary::self()->items() )
    {
        if( !item->createItemFnPtr() ) continue; // Is category
        if( item->type() != type ) continue;

        comp = item->createItemFnPtr()( type, id );
        break;
    }
    if( map ) m_compMap[id] = comp;
    return comp;
}

void Circuit::removeItems()                     // Remove Selected items
{
    if( m_conStarted ) return;
    m_busy = true;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    beginUndoStep(); // Save current state

    QList<Connector*> conns;
    QList<Component*> comps;

    for( QGraphicsItem* item : selectedItems() )    // Find all items to be removed
    {
        if( item->type() == QGraphicsItem::UserType+2 )      // ConnectorLine: add Connector to list
        {
            ConnectorLine* line = qgraphicsitem_cast<ConnectorLine*>( item );
            Connector* con = line->connector();
            if( !conns.contains( con ) && m_oldConns.contains( con ) ) conns.append( con );
        }
        else if( item->type() == QGraphicsItem::UserType+1 ) // Component: add Component to list
        {
            Component* comp = qgraphicsitem_cast<Component*>( item );
            if( m_oldComps.contains( comp ) ) comps.append( comp );
        }
    }
    for( Connector* conn : conns ) removeConnector( conn );         // Remove Connectors (does not delete)
    for( Component* comp: comps ){
        if( comp->itemType() == "Node" ) removeNode( (Node*)comp ); // Remove Nodes (does not delete)
        else                             removeComp( comp );        // Remove Components (does not delete)
    }
    endUndoStep();                                           // Calculates items actually created/removed

    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
    m_busy = false;
}

void Circuit::removeComp( Component* comp )
{
    //if( comp->parentItem() ) return; // subcircuit
    m_compRemoved = false;
    comp->remove();
    if( !m_compRemoved ) return;

    if( m_compList.contains( comp ) ) m_compList.removeOne( comp );
    removeItem( comp );
    m_compMap.remove( comp->getUid() );
    m_removedComps.append( comp );
}

void Circuit::removeNode( Node* node )
{
    if( !m_nodeList.contains(node) ) return;
    if( m_deleting ) return;
    m_nodeList.removeOne( node );
    m_compMap.remove( node->getUid() );
    removeItem( node );
    m_removedComps.append( node );
}

void Circuit::removeConnector( Connector* conn )
{
    if( !m_connList.contains(conn) ) return;
    conn->remove();
    m_connList.removeOne( conn );
    m_compMap.remove( conn->getUid() );
    m_removedComps.append( conn );
}

void Circuit::clearCircuit() // Remove everything ( Clear Circuit )
{
    if( m_conStarted ) return;
    m_deleting = true;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    for( Component* comp : m_compList )
    {
        //if( comp->parentItem() ) continue; // Shields are deleted with parent if attached
        comp->remove();
        if( comp->scene() ) removeItem( comp );
        delete comp;
    }
    for( Node* node : m_nodeList )
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

void Circuit::saveChanges()
{
    if( m_conStarted || m_circChange.size() == 0 ) return;
    setChanged();

    while( m_undoStack.size() > (m_undoIndex+1) ) m_undoStack.removeLast();

    m_undoStack.append( m_circChange );
    if( m_undoStack.size() > m_maxUndoSteps )
    {
        m_undoStack.takeFirst();
        m_undoIndex--;
    }
    m_undoIndex++;

    clearCircChanges();
    m_cicuitBatch = 0;  // Ends all CicuitChanges
    deleteRemoved();    // Delete Removed Components;

    /// qDebug() << "Circuit::saveChanges ---------------------------"<<m_undoIndex<<m_undoStack.size()<<endl;
}

void Circuit::deleteRemoved()
{
    for( CompBase* comp : m_removedComps ) delete comp;
    m_removedComps.clear();
}

void Circuit::removeLastUndo()
{
    if( m_undoStack.isEmpty() ) return;
    m_undoStack.takeLast();
    m_undoIndex--;
}

void Circuit::beginCircuitBatch() // Don't create/remove
{
    /// qDebug() << "Circuit::beginCircuitBatch";
    if( !m_cicuitBatch ) clearCircChanges();
    m_cicuitBatch++;
}

void Circuit::endCircuitBatch() // Don't create/remove
{
    /// qDebug() << "Circuit::endCircuitBatch";
    if( m_cicuitBatch > 0 ){
        m_cicuitBatch--;
        if( m_cicuitBatch == 0 ) saveChanges();
    }
}

void Circuit::cancelUndoStep()
{
    calcCircuitChanges();
    if( m_circChange.size() )
    {
        endCircuitBatch();
        undo();
        m_undoStack.takeLast();
    }
    else m_cicuitBatch = 0;
    /// qDebug() << "Circuit::cancelUndoStep--------------------------------"<<endl;
}

void Circuit::beginUndoStep() // Save current state
{
    beginCircuitBatch();

    m_oldConns = m_connList;
    m_oldComps = m_compList;
    m_oldNodes = m_nodeList;
    m_compStrMap.clear();      /// FIXME: optimize this, we are saving the whole circuit every time

    //save all comps
    for( Connector* conn : m_oldConns ) m_compStrMap.insert( conn, conn->toString() );
    for( Node*      node : m_oldNodes ) m_compStrMap.insert( node, node->toString() );
    for( Component* comp : m_oldComps ) m_compStrMap.insert( comp, comp->toString() );
}

void Circuit::endUndoStep()   //
{
    calcCircuitChanges();
    endCircuitBatch();
}

void Circuit::calcCircuitChanges()   // Calculates created/removed
{
    /// qDebug() << "Circuit::calcCicuitChanges Removed:";
    // Items Removed
    QList<Connector*> removedConns = substract( m_oldConns, m_connList );
    QList<Node*>      removedNodes = substract( m_oldNodes, m_nodeList );
    QList<Component*> removedComps = substract( m_oldComps, m_compList );

    for( Connector* conn : removedConns ) addCompChange( conn->getUid(), COMP_STATE_NEW, m_compStrMap.value(conn) );
    for( Node*      node : removedNodes ) addCompChange( node->getUid(), COMP_STATE_NEW, m_compStrMap.value(node) );
    for( Component* comp : removedComps ) addCompChange( comp->getUid(), COMP_STATE_NEW, m_compStrMap.value(comp) );

    /// qDebug() << "Circuit::calcCicuitChanges Created:";
    // Items Created
    QList<Connector*> createdConns = substract( m_connList, m_oldConns );
    QList<Node*>      createdNodes = substract( m_nodeList, m_oldNodes );
    QList<Component*> createdComps = substract( m_compList, m_oldComps );

    for( Component* comp : createdComps ) addCompChange( comp->getUid(), COMP_STATE_NEW, "" );
    for( Node*      node : createdNodes ) addCompChange( node->getUid(), COMP_STATE_NEW, "" );
    for( Connector* conn : createdConns ) addCompChange( conn->getUid(), COMP_STATE_NEW, "" );
}

void Circuit::saveCompChange( QString component, QString property, QString undoVal )
{
    clearCircChanges();
    addCompChange( component, property, undoVal );
    saveChanges();
}

void Circuit::addCompChange( QString component, QString property, QString undoVal )
{
    if( m_loading || m_deleting ) return;                      /// qDebug() << "Circuit::addCompChange      " << component << property;// << value;
    compChange cChange{ component, property, undoVal, "" };
    m_circChange.compChanges.append( cChange );
}

void Circuit::restoreState()
{
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_busy = true;

    circChange& step = m_undoStack[ m_undoIndex ];

    int iStep, i;
    if( m_undo ) { iStep = -1; i = step.compChanges.size()-1; }
    else         { iStep =  1; i = 0; }

    while( i>=0 && i < step.compChanges.size() )
    {
        compChange* cChange = &step.compChanges[i];
        i += iStep;
        QString propName   = cChange->property;
        QString propVal    = m_undo ? cChange->undoValue : cChange->redoValue;
        CompBase* comp     = m_compMap.value( cChange->component );             /// qDebug() << "Circuit::restoreState"<< cChange->component << propName << comp;

        if( propName == COMP_STATE_NEW )  // Create/Remove Item
        {
            if( propVal.isEmpty() )       // Remove item
            {
                if( !comp ) continue;
                if( m_undo && cChange->redoValue.isEmpty() ) cChange->redoValue = comp->toString();

                if     ( comp->itemType() == "Connector" ) removeConnector( (Connector*)comp );
                else if( comp->itemType() == "Node"      ) removeNode( (Node*)comp );
                else                                       removeComp( (Component*)comp );
            }
            else loadStrDoc( propVal );   // Create Item
        }
        else if( comp )                   // Modify Property
        {
            if( m_undo && cChange->redoValue.isEmpty() ) cChange->redoValue = comp->getPropStr( propName );
            comp->setPropStr( propName, propVal );
        }
    }
    m_busy = false;
    deleteRemoved();                      // Delete Removed Components;
    for( Connector* con : m_connList ) {
        if( m_board && m_board->m_boardMode ) con->setVisib( false );
        else{
            con->startPin()->isMoved();
            con->endPin()->isMoved();
        }
    }
    update();
}

void Circuit::undo()
{
    if( m_busy || m_deleting || m_conStarted || m_undoIndex < 0 ) return; /// qDebug() << "\nCircuit::undo"<<m_undoIndex<<m_undoStack.size();
    m_undo = true;
    restoreState();
    m_undoIndex--;
    m_undo = false;
}

void Circuit::redo()
{
    if( m_busy || m_deleting || m_conStarted || m_undoIndex >= (m_undoStack.size()-1) ) return;
    m_redo = true;
    m_undoIndex++;                                 /// qDebug() << "\nCircuit::redo"<<m_undoIndex<<m_undoStack.size();
    restoreState();
    m_redo = false;
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
            if( comp && !comp->isHidden() && !comp->parentItem() )
            {
                complist.append( comp );
            }
        }
        else if( item->type() == QGraphicsItem::UserType+2 ) // ConnectorLine
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( item );
            Connector* con = line->connector();
            if( !conlist.contains( con ) ) conlist.append( con );
    }   }
    QString circuit;
    for( CompBase* comp : complist ) circuit += comp->toString();
    for( CompBase* con  :  conlist ) circuit += con->toString();

    QApplication::clipboard()->setText( circuit );
}

void Circuit::paste( QPointF eventpoint )
{
    if( m_conStarted ) return;
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    m_busy    = true;
    m_pasting = true;

    if( m_eventpoint == QPointF(1e6,1e6) ) // We don't have origin point
    {
        QRectF itemsRect = selectionArea().boundingRect();
        m_eventpoint = toGrid( itemsRect.topLeft() );
    }
    if( eventpoint == QPointF(1e6,1e6) ) eventpoint = m_eventpoint+QPointF( 8, 8 );

    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    m_deltaMove = toGrid(eventpoint) - m_eventpoint;

    QString circuit = QApplication::clipboard()->text();

    beginUndoStep();
    loadStrDoc( circuit );
    endUndoStep();

    m_pasting = false;
    m_busy = false;
}

void Circuit::newconnector( Pin* startpin, bool save )
{
    if( save ) beginUndoStep();

    m_conStarted = true;

    QString type = QString("Connector");
    QString id = type+"-"+newConnectorId() ;

    m_newConnector = new Connector( type, id, startpin );
    m_connList.append( m_newConnector );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();
    m_newConnector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );
}

void Circuit::closeconnector( Pin* endpin, bool save )
{
    m_conStarted = false;
    m_newConnector->closeCon( endpin );
    if( save ) endUndoStep();
}

void Circuit::deleteNewConnector()
{
    if( !m_conStarted ) return;
    removeConnector( m_newConnector );
    m_conStarted = false;

    cancelUndoStep();
}

/*void Circuit::updateConnectors()
{ for( Connector* con : m_connList ) con->updateLines(); }*/

void Circuit::addNode( Node* node )
{
    addItem( node );
    m_nodeList.append( node );
    m_compMap.insert( node->getUid(), node );
}

void Circuit::addComponent( Component* comp )
{
    addItem( comp );
    if( comp->itemType() == "Package" ) m_compList.prepend( comp );
    else                                m_compList.append( comp );
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
        if( m_conStarted ) event->accept();
        QGraphicsScene::mousePressEvent( event );

        if( !event->isAccepted() ) Linker::stopLinking(); // Click in empty place
    }
    else if( event->button() == Qt::RightButton )
    {
        if( m_conStarted ) event->accept();
        else               QGraphicsScene::mousePressEvent( event );
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
        if( event->modifiers() & Qt::ShiftModifier) m_newConnector->m_freeLine = true;
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
    if( key == Qt::Key_Escape )
    {
        Linker::stopLinking();
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
        QString compNam = ComponentList::self()->getComponent( str );
        if( !compNam.isEmpty() )
        {
            Component* enterItem = createItem( compNam, compNam+"-"+newSceneId() );
            if( enterItem )
            {
                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( toGrid( CircuitView::self()->mapToScene( cPos ) ) );
                addComponent( enterItem );
                saveCompChange( enterItem->getUid(), COMP_STATE_NEW, "" );
            }
        }
    }
    if( event->modifiers() & Qt::ControlModifier )
    {
        if( key == Qt::Key_C )
        {
            //QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            //copy( m_graphicView->mapToScene( p ) );
            copy( QPointF(1e6,1e6) ); // We don't have an origin
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
            //QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            //paste( m_graphicView->mapToScene( p ) );
            paste( QPointF(1e6,1e6) ); // We don't have a destination
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
            for( Connector* con : m_connList ) con->setSelected( true );
        }
        else if( key == Qt::Key_R )
        {
            if ( event->modifiers() & Qt::ShiftModifier ) {
                for( Component* com : m_compList ) if( com->isSelected() ) com->rotateCCW();
            } else {
                for( Component* com : m_compList ) if( com->isSelected() ) com->rotateCW();
            }
        }
        else if( key == Qt::Key_L )
        {
            if ( event->modifiers() & Qt::ShiftModifier ) {
                for( Component* com : m_compList ) if( com->isSelected() ) com->slotV_flip();
            } else {
                for( Component* com : m_compList ) if( com->isSelected() ) com->slotH_flip();
            }
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
//qDebug() << "Circuit::dropEvent";
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
            Component* enterItem = createItem( "Image", newSceneId() );
            if( enterItem )
            {
                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( CircuitView::self()->mapToScene( cPos ) );
                enterItem->setBackground( file );
                addComponent( enterItem );
                saveCompChange( enterItem->getUid(), COMP_STATE_NEW, "" );
        }   }
        else CircuitWidget::self()->loadCirc( id );
}
}

void Circuit::drawBackground( QPainter* painter, const QRectF &rect )
{
    /*painter->setBrush(QColor( 255, 255, 255 ) );
    painter->drawRect( m_scenerect );
    return;*/

    painter->setBrush( QColor( 240, 240, 210 ) );
    painter->drawRect( m_scenerect );
    painter->setPen( QColor( 210, 210, 210 ) );

    if( m_hideGrid ) return;

    int startx = m_scenerect.x();
    int endx   = m_scenerect.width()/2;
    int starty = m_scenerect.y();
    int endy   = m_scenerect.height()/2;

    int scnStrX = rect.x()-16;
    if( scnStrX < startx ) scnStrX = startx;

    int scnEndX = scnStrX+rect.width()+32;
    if( scnEndX > endx ) scnEndX = endx;

    int scnStrY = rect.y()-16;
    if( scnStrY < starty ) scnStrY = starty;

    int scnEndY = scnStrY+rect.height()+32;
    if( scnEndY > endy ) scnEndY = endy;

    for( int i=4; i<endx; i+=8 ){
        if( i > scnEndX && -i < scnStrX) break;
        if(  i < scnEndX ) painter->drawLine( i, scnStrY, i, scnEndY );
        if( -i > scnStrX ) painter->drawLine(-i, scnStrY,-i, scnEndY );
    }
    for( int i=4; i<endy; i+=8 ){
        if( i > scnEndY && -i < scnStrY) break;
        if(  i < scnEndY ) painter->drawLine( scnStrX, i, scnEndX, i);
        if( -i > scnStrY ) painter->drawLine( scnStrX,-i, scnEndX,-i);
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
