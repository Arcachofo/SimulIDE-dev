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
    QList<Component*> compList;   // Pasting Component list
    QList<Connector*> conList;    // Pasting Connector list
    QList<Node*> nodeList;        // Pasting node list

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
                if( propName == "MainCompId") mComp = subci->getMainComp( prop.toString() );  // If more than 1 mainComp then get Component
                else                          mComp->setPropStr( propName, prop.toString() );

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
                if( endpin   && endpin->connector() )   endpin   = NULL;

                if( startpin && endpin )    // Create Connector
                {
                    if( newUid.isEmpty() ) newUid = "connector-"+newConnectorId();
                    Connector* con = new Connector( this, type, newUid, startpin, endpin );
                    con->setPointList( pointList );
                    conList.append( con );
                    m_newComp = con;
                    if( !rev ){
                        startpin->isMoved();
                        endpin->isMoved();
                    }
                    int number = newUid.split("-").last().toInt();
                    if( number > m_conNumber ) m_conNumber = number; // Adjust Connector counter: m_conNumber
                }
                else if( !m_pasting && !m_undo && !m_redo )// Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null startpin in Connector" << uid << startpinid;
                    if( !endpin   ) qDebug() << "\n   ERROR!!  Circuit::loadStrDoc:  null endpin in Connector"   << uid << endpinid;
            }   }
            else if( type == "Node")
            {
                Node* joint = new Node( this, type, newUid );
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
                    compList.append( comp );
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

    m_compList.append( compList );
    m_nodeList.append( nodeList );
    m_conList.append( conList );

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
    for( Connector* conn : m_conList )  circuit += conn->toString();
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

        comp = item->createItemFnPtr()( this, type, id );
    }
    m_compMap[id] = comp;
    return comp;
}

void Circuit::removeItems()                     // Remove Selected items
{
    if( m_conStarted ) return;
    m_busy = true;

    QList<Connector*> connectors;
    QList<Component*> components;
    addCompState( NULL, "New State", stateNew );

    for( QGraphicsItem* item : selectedItems() )
    {
        if( item->type() == QGraphicsItem::UserType+1 ) // Component
        {
            Component* comp = qgraphicsitem_cast<Component* >( item );
            if( m_compList.contains( comp )
            && !components.contains( comp ) )
            {
                components.append( comp );
                addCompState( comp, "new", stateAdd );

                for( Pin* pin : comp->getPins() )
                {
                    if( !pin ) continue;
                    Connector* con = pin->connector();
                    if( con ) addCompState( con, "new", stateAdd );
                }
            }
        }
        else if( item->type() == QGraphicsItem::UserType+2 ) // ConnectorLine
        {
            ConnectorLine* line = qgraphicsitem_cast<ConnectorLine* >( item );
            Connector* con = line->connector();
            if( !connectors.contains( con ) )
            {
                connectors.append( con );
                addCompState( con, "new", stateAdd );
            }
    }   }
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    for( Connector* con  : connectors ) con->remove();
    for( Component* comp : components ) removeComp( comp );
    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
    saveState();
    m_busy = false;
}

void Circuit::removeComp( Component* comp )
{
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_compRemoved = false;
    comp->remove();
    if( !m_compRemoved ) return;

    if( m_compList.contains( comp ) ) m_compList.removeOne( comp );
    if( comp->scene() ) removeItem( comp );
    m_compMap.remove( comp->getUid() );
    delete comp;
}

void Circuit::clearCircuit() // Remove everything ( Clear Circuit )
{
    if( m_conStarted ) return;
    m_deleting = true;

    while( !m_compList.isEmpty() ) removeComp( m_compList.takeFirst() );
    while( !m_nodeList.isEmpty() )
    {
        Node* nod = m_nodeList.takeFirst();
        if( nod->scene() ) removeItem( nod );
        delete nod;
    }
    m_pinMap.clear();
    m_deleting = false;
}

void Circuit::deselectAll()
{ for( QGraphicsItem* item : selectedItems() ) item->setSelected( false ); }

void Circuit::saveBackup()
{
    if( m_simulator->isRunning() ) return;
    if( !m_changed || m_conStarted || m_busy || m_deleting ) return;
    m_changed = false;
    m_busy = true;

    saveString( m_backupPath, circuitToString() ); // Backup file
    m_busy = false;
}

void Circuit::saveState()
{
    if( m_conStarted || m_circState.size() == 0 ) return;
    setChanged();

    if( m_undo )
    {
        m_redoStack.append( m_circState );
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
//qDebug() << "Circuit::saveState Undo:"<< m_undoStack.size()<<m_undoIndex
//                              <<"Redo:"<<m_redoStack.size()<<m_redoIndex<<"\n";

}

void Circuit::setChanged()
{
    m_changed = true;
    QString title = MainWindow::self()->windowTitle();
    if( !title.endsWith('*') ) MainWindow::self()->setWindowTitle(title+'*');
}

void Circuit::unSaveState()
{
    if( m_undoStack.isEmpty() ) return;
    m_undoStack.takeLast();
    m_undoIndex--;
}

void Circuit::addCompState( CompBase* c, QString p, stateMode state )
{
    if( m_loading || m_deleting ) return;
    QString type = "NULL";
    if( c ) type = c->itemType();
//qDebug() << "Circuit::addCompState" << type<<p<<state;
//qDebug() <<(state&stateNew)<<(state&stateAdd)<<(state&stateSave)<<((int)state&(int)stateSave);
    if( state & stateNew )
    {
        m_circState.clear();
    }
    if( state & stateAdd )
    {
        compState cState;
        cState.component = c->getUid();
        cState.property  = p;
        if( p == "new" ) cState.valStr = c->toString();
        else             cState.valStr = c->getPropStr( p );

        if( c->itemType() == "Connector" )
        {
            Connector* con = (Connector*)c;
            if( p == "remove" )
            {
                cState.valStr = con->startPinId(); // Save Connector start Pin id.
                m_circState.remove.prepend( cState );
            }
            else m_circState.create.append( cState );
        }
        else               // Component
        {
            if( p == "remove" ) m_circState.remove.append( cState );
            else                m_circState.create.prepend( cState );
        }
    }
    if( state & stateSave )
    {
//qDebug() << "Circuit::addCompState saveState";
        saveState();
    }
}

void Circuit::undo()
{
    if( m_busy || m_deleting || m_conStarted || m_undoIndex < 0 ) return;
//qDebug() << "\nCircuit::undo"<<m_undoIndex<<m_redoIndex;
    m_undo = true;
    if( m_undoIndex > m_undoStack.size() )
    {
        qDebug() << "Circuit::redo index Error"<< m_undoStack.size()<<m_undoIndex;
        clearUndoRedo();
        return;
    }
    circState step = m_undoStack.at( m_undoIndex );
    if( restoreState( step ) )
    {
        m_undoIndex--;
        m_redoIndex++;
        saveState();

        if( m_board && m_board->m_boardMode )
            for( Connector* con : m_conList  ) con->setVisib( false );
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
//qDebug() << "\nCircuit::redo"<<m_undoIndex<<m_redoIndex;
    m_redo = true;
    if( m_redoIndex > m_redoStack.size() )
    {
        qDebug() << "Circuit::redo index Error"<< m_redoStack.size()<<m_redoIndex;
        clearUndoRedo();
        return;
    }
    circState step = m_redoStack.at( m_redoIndex );
    if( restoreState( step ) )
    {
        m_redoIndex--;
        m_undoIndex++;
        if( m_redoIndex < 0 ) m_redoStack.clear();

        if(  m_board && m_board->m_boardMode )
            for( Connector* con : m_conList  ) con->setVisib( false );
    }
    else clearUndoRedo();
    m_redo = false;

//qDebug() << "Circuit::redo Undo:"<< m_undoStack.size()<<m_undoIndex
//                        <<"Redo:"<<m_redoStack.size()<<m_redoIndex<<"\n";
//qDebug() << "Circuit::redo-----------------------\n";
}

bool Circuit::restoreState( circState step )
{
    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_busy = true;

    if( m_undo ) addCompState( NULL, "", stateNew ); // Start new Undo/Redo state

    for( compState cState : step.remove )
    {
        QString compName = cState.component;
        CompBase* comp   = m_compMap.value( compName );
//qDebug() << "Circuit::restoreState"<<compName<<cState.property;

        if( comp && comp->itemType() != "Connector" )
        {
            if( m_undo ) addCompState( comp, "new", stateAdd ); // Add compState to new Redo state
            if( comp->itemType() == "Node")
            {
                Node* node = (Node*)comp;
                if( m_nodeList.contains( node ) ) m_nodeList.removeOne( node );
                if( node->scene() ) removeItem( node );
                m_compMap.remove( node->getUid() );
                delete node;
            }
            else removeComp( (Component*)comp );
        }else{
            QString propVal  = cState.valStr;
            Pin* pin = m_pinMap.value( propVal ); // Find Connector start Pin from id.
            if( !pin ){
                //qDebug() << "Circuit::restoreState Error NULL Pin"<<compName<<propVal;
                continue; //return false;
            }
            Connector* con = pin->connector();
            if( !con ){
                //qDebug() << "Circuit::restoreState Error NULL Connector"<<compName<<propVal;
                continue; // return false;
            }
            if( m_undo )
            {
                addCompState( con, "new", stateAdd ); // Add compState to new Redo state
                con->remove();
            }
            else
            {
                conList()->removeOne( con );
                con->remLines();
                con->startPin()->setConnector( NULL );
                con->endPin()->setConnector( NULL );
            }
        }
    }
    for( compState cState : step.create )
    {
        QString compName = cState.component;
        QString propName = cState.property;
        QString propVal  = cState.valStr;
        CompBase* comp   = m_compMap.value( compName );
//qDebug() << "Circuit::restoreState"<<compName<<propName;
        if( propName == "new" )
        {
            loadStrDoc( propVal );
            if( m_undo && m_newComp ) addCompState( m_newComp, "remove", stateAdd );
        }
        else // Property
        {
            if( !comp ){
                qDebug() << "Circuit::restoreState Error restoring"<<compName<<propVal;
                continue;
            }
            if( m_undo ) addCompState( comp, propName, stateAdd );
            comp->setPropStr( propName, propVal );
        }
    }
    m_LdPinMap.clear();
    m_busy = false;
    update();
    return true;
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

    m_eventpoint = toGrid(eventpoint);

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
    saveState();
    m_busy = true;
    m_pasting = true;
    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    m_deltaMove = toGrid(eventpoint) - m_eventpoint;

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
    addCompState( NULL, "", stateNew );
    m_conStarted = true;

    QString type = QString("Connector");
    QString id = type+"-"+newConnectorId() ;

    m_newConnector = new Connector( this, type, id, startpin );
    m_conList.append( m_newConnector );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();
    m_newConnector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );
}

void Circuit::closeconnector( Pin* endpin )
{
    m_conStarted = false;
    m_newConnector->closeCon( endpin );
    addCompState( m_newConnector, "remove", stateAdd );
    saveState();
}

void Circuit::deleteNewConnector()
{
    if( !m_conStarted ) return;
    m_newConnector->remove();
    m_conStarted = false;
    unSaveState();
}

void Circuit::updateConnectors()
{ for( Connector* con : m_conList ) con->updateLines(); }

void Circuit::addNode( Node* node )
{
    addItem( node );
    m_nodeList.append( node );
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
    if( event->modifiers() & Qt::AltModifier )
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
                m_compList.append( enterItem );
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
            for( Connector* con : m_conList  ) con->setSelected( true );
        }
        else if( key == Qt::Key_R )
        {
            for( Component* com : m_compList ) if (com->isSelected()) com->rotateCW();
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
