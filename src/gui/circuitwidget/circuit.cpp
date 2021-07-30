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

#include "circuit.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "connectorline.h"
#include "simuapi_apppath.h"
#include "node.h"
#include "utils.h"
#include "subcircuit.h"
#include "mcucomponent.h"
#include "simulator.h"

static const char* Circuit_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Speed"),
    QT_TRANSLATE_NOOP("App::Property","ReactStep"),
    QT_TRANSLATE_NOOP("App::Property","NoLinAcc"),
    QT_TRANSLATE_NOOP("App::Property","Draw Grid"),
    QT_TRANSLATE_NOOP("App::Property","Show ScrollBars"),
    QT_TRANSLATE_NOOP("App::Property","Animate"),
    QT_TRANSLATE_NOOP("App::Property","Font Scale"),
    QT_TRANSLATE_NOOP("App::Property","Auto Backup Secs")
    QT_TRANSLATE_NOOP("App::Property","Language")
};

Circuit*  Circuit::m_pSelf = 0l;

Circuit::Circuit( qreal x, qreal y, qreal width, qreal height, QGraphicsView*  parent )
       : QGraphicsScene( x, y, width, height, parent )
{
    Q_UNUSED( Circuit_properties );

    m_simulator = new Simulator();
    
    setObjectName( "Circuit" );
    setParent( parent );
    m_graphicView = parent;
    m_scenerect.setRect( x, y, width, height );
    setSceneRect( QRectF(x, y, width, height) );

    m_pSelf = this;
    m_circType = "simulide_0.5";

    m_busy       = false;
    m_changed    = false;
    m_animate    = false;
    m_pasting    = false;
    m_deleting   = false;
    m_conStarted = false;

    new_connector = 0l;
    m_seqNumber   = 0;
    
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

    // Clear Undo/Redo stacks
    for( QDomDocument* doc : m_redoStack ) delete doc;
    for( QDomDocument* doc : m_undoStack ) delete doc;
    m_undoStack.clear();
    m_redoStack.clear();

    if( !m_backupPath.isEmpty() )
    {
        QFile file( m_backupPath );
        if( !file.exists() ) return;
        QFile::remove( m_backupPath ); // Remove backup file
}   }

Component* Circuit::getComponent( QString name )
{
    for( Component* comp : m_compList ) if( comp->idLabel() == name ) return comp;
    return 0l;
}

Component* Circuit::getCompById( QString id )
{
    for( Component* comp : m_compList ) if( comp->objectName() == id ) return comp;
    return 0l;
}

QString Circuit::getCompId( QString name )
{
    //return name.remove( name.lastIndexOf("-"), 100 );

    QStringList nameSplit = name.split("-");
    if( nameSplit.isEmpty() ) return "";

    QString compId  = nameSplit.takeFirst();
    if( nameSplit.isEmpty() ) return "";

    QString compNum = nameSplit.takeFirst();

    return compId+"-"+compNum;
}

Pin* Circuit::findPin( int x, int y, QString id )
{
    qDebug() << "Circuit::findPin" << id;
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
    return 0l;
}

void Circuit::loadCircuit( QString fileName )
{
    if( m_conStarted ) return;
    saveState();

    m_filePath = fileName;
    QDomDocument domDoc = fileToDomDoc( fileName, "Circuit::loadCircuit");
    if( domDoc.isNull() ) return;

    m_error = 0;
    loadDomDoc( &domDoc );
    
    if( m_error != 0 ) 
    {
        remove();
        for( Component* comp : m_compList ) removeComp( comp ); // Clean Nodes
    }
    else
    {
        m_graphicView->centerOn( QPointF(
            width()/2  + itemsBoundingRect().center().x() - m_graphicView->width()/2,
            height()/2 + itemsBoundingRect().center().y() - m_graphicView->height()/2 ) );
        qDebug() << "Circuit Loaded: ";
        qDebug() << fileName;
}   }

void Circuit::loadDomDoc( QDomDocument* doc )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Component*> compList;   // Component List
    QList<Component*> conList;    // Connector List
    QList<Node*>      jointList;  // Joint List
    QList<SubCircuit*> shieldList;

    QHash<QString, eNode*> nodMap;

    m_busy    = true;
    m_animate = false;

    QDomElement circuit = doc->documentElement();
    m_circType = circuit.attribute("type");

    if( circuit.hasAttribute( "stepsPS" )) m_simulator->setStepsPerSec( circuit.attribute("stepsPS").toDouble() );
    if( circuit.hasAttribute( "NLsteps" )) m_simulator->setMaxNlSteps( circuit.attribute("NLsteps").toUInt() );
    if( circuit.hasAttribute( "stepSize" )) m_simulator->setStepSize( circuit.attribute("stepSize").toULongLong() );
    if( circuit.hasAttribute( "animate" )) setAnimate( circuit.attribute("animate").toInt() );

    QDomNode node = circuit.firstChild();

    while( !node.isNull() )
    {
        QDomElement   element = node.toElement();
        const QString tagName = element.tagName();

        if( tagName == "item" )
        {
            QString objNam = element.attribute( "objectName"  ); // Data in simu file
            QString type   = element.attribute( "itemtype"  );
            QString id     = element.attribute( "id"  );//objNam.split("-").first()+"-"+newSceneId(); // Create new id

            if( type == "Connector" )
            {
                Pin* startpin  = 0l;
                Pin* endpin    = 0l;
                QString startpinid    = element.attribute( "startpinid" );
                QString endpinid      = element.attribute( "endpinid" );

                startpin = m_pinMap[startpinid];
                endpin   = m_pinMap[endpinid];

                if( !startpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.first().toInt();
                    int itemY = pointList.at(1).toInt();

                    startpin = findPin( itemX, itemY, startpinid );
                }
                if( !endpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.at(pointList.size()-2).toInt();
                    int itemY = pointList.last().toInt();

                    endpin = findPin( itemX, itemY, endpinid );
                }

                if( startpin && startpin->isConnected() ) startpin = 0l;
                if( endpin   && endpin->isConnected() )   endpin   = 0l;

                if( startpin && endpin )    // Create Connector
                {
                    Connector* con  = new Connector( this, type, id, startpin, endpin );

                    element.setAttribute( "startpinid", startpin->objectName() );
                    element.setAttribute(   "endpinid", endpin->objectName() );

                    loadProperties( element, con );

                    QString enodeId = element.attribute( "enodeid" );
                    eNode*  enode   = nodMap[enodeId];
                    if( !enode )                    // Create eNode and add to enodList
                    {
                        enode = new eNode( "Circ_eNode-"+newSceneId() );
                        nodMap[enodeId] = enode;
                    }
                    con->setEnode( enode );

                    QStringList plist = con->pointList();   // add lines to connector
                    int count = plist.size();
                    if( count < 2 )
                    {
                        qDebug() << "Error creating Connector: empty pointList ";
                    }
                    else
                    {
                        int p1x = plist.first().toInt();
                        int p1y = plist.at(1).toInt();
                        int p2x = plist.at(plist.size()-2).toInt();
                        int p2y = plist.last().toInt();

                        con->addConLine( con->x(),con->y(), p1x, p1y, 0 );

                        for (int i=2; i<count; i+=2)
                        {
                            p2x = plist.at(i).toInt();
                            p2y = plist.at(i+1).toInt();
                            con->addConLine( p1x, p1y, p2x, p2y, i/2 );
                            p1x = p2x;
                            p1y = p2y;
                        }
                        con->updateConRoute( startpin, startpin->scenePos() );
                        con->updateConRoute( endpin, endpin->scenePos() );
                        con->remNullLines();
                        conList.append( con );
                        startpin->registerPins( enode );
                        endpin->registerPins( enode );
                }  }
                else // Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null startpin in " << objNam << startpinid;
                    if( !endpin )   qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null endpin in "   << objNam << endpinid;
            }   }
            else if( type == "Node")
            {
                Node* joint = new Node( this, type, objNam );
                loadProperties( element, joint );
                compList.append( joint );
                jointList.append( joint );
            }
            else if( (type == "Plotter") && id.contains("Widget") ) ;// Old Plotter widget;
            else if(( type == "SerialPort")
                 && ( element.hasAttribute( "visible" ) )
                 && ( element.attribute( "visible" ) == "false" ) ) ;// Old Serial Port Widget
            else
            {
                bool oldArduino = false;
                if( type == "Arduino" )
                {
                    oldArduino = true;
                    type = "Subcircuit";
                    id = id.remove( "Arduino " );
                    objNam = objNam.remove( "Arduino " );
                }
                Component* item = createItem( type, objNam, objNam );

                if( item )
                {
                    loadProperties( element, item );

                    if( oldArduino ) // Load mcu properties & change subcircuit names
                    {
                        SubCircuit* subci = static_cast<SubCircuit*>(item);
                        McuComponent* mcu = static_cast<McuComponent*>( subci->getMainComp() );
                        mcu->setSubcDir("");
                        mcu->setProgram( element.attribute("Program") );
                        mcu->setFreq( element.attribute("Mhz").toDouble() );
                        mcu->setAutoLoad( element.attribute("Auto_Load").toInt() );

                        subci->setObjectName( subci->objectName().remove( "Arduino " ) );
                        subci->setId( subci->itemID().remove( "Arduino " ) );
                    }
                    else if( item->itemType() == "Subcircuit")
                    {
                        SubCircuit* shield = static_cast<SubCircuit*>(item);
                        if( shield->subcType() == Chip::subcShield )shieldList.append( shield );
                    }
                    compList.append( item );
                }
                else qDebug() << " ERROR Creating Component: "<< type << objNam;
        }   }
        node = node.nextSibling();
    }
    // Take care about unconnected Joints
    for( Node* joint : jointList ) joint->remove(); // Only removed if some missing connector
    for( SubCircuit* shield : shieldList ) shield->connectBoard();

    m_busy = false;
    QApplication::restoreOverrideCursor();
}

void Circuit::loadProperties( QDomElement element, Component* comp )
{
    if( element.hasChildNodes() )
    {
        QDomNode nod = element.childNodes().at(0);
        QDomElement el = nod.toElement();
        if( el.tagName() == "mainCompProps") // Load Subcircuit Main Component properties
        {
            if( comp->itemType() == "Subcircuit" )
            {
                SubCircuit* subc = (SubCircuit*)comp;
                Component* mainComp = subc->getMainComp();
                if( mainComp ) loadObjectProperties( el, mainComp );
                element.removeChild( nod );
    }   }   }

    loadObjectProperties( element, comp );

    comp->setLabelPos();
    comp->setValLabelPos();
    addItem( comp );

    int number = comp->objectName().split("-").last().toInt();
    if ( number > m_seqNumber ) m_seqNumber = number;               // Adjust item counter: m_seqNumber
}

void Circuit::loadObjectProperties( QDomElement element, Component* comp )
{
    QDomNamedNodeMap atrs = element.attributes();

    for( int i=0; i<atrs.length(); ++i )   // Get List of property names in Circuit file
    {
        QString propName = atrs.item(i).nodeName();
        if( propName == "mainCompProps") // Load Subcircuit Main Component properties
        {
            if( comp->itemType() == "Subcircuit" )
            {
                SubCircuit* subc = (SubCircuit*)comp;
                Component* mainComp = subc->getMainComp();
                if( mainComp ) loadObjectProperties( atrs.item(i).toElement(), mainComp );
                continue;
        }   }
        QVariant value( element.attribute( propName ) );
        QVariant comProp = comp->property( propName.toStdString().c_str() );

        // SUBSTITUTIONS -------------------------------------------------------
        if( !comProp.isValid() )
        {
            if     ( propName == "Volts")   propName = "Voltage";
            else if( propName == "Current") propName = "Value";
            else if( propName == "Propagation_Delay_ns")
            {
                propName = "Tpd_ps";
                value.setValue( value.toInt()*1000 );
            }
            else if( propName == "Show_res"
                  || propName == "Show_Amp"
                  || propName == "Show_Ind"
                  || propName == "Show_Ind"
                  || propName == "Show_Cap" ) propName = "Show_Val";
            else if( propName == "Duty_Square") propName = "Duty";
            else if( propName == "Inverted") propName = "InvertOuts";

            comProp = comp->property( propName.toStdString().c_str() );
        }

        if( !value.isValid()
         || !comProp.isValid() )
        {
            qDebug() << "loadObjectProperties Wrong Property: "<< comp->itemID() << propName << propName.toStdString().c_str();
            continue;
        }
        QVariant::Type type = comProp.type();

        if     ( type == QVariant::Int    ) comp->setProperty( propName.toStdString().c_str(), value.toInt() );
        else if( type == QVariant::Double ) comp->setProperty( propName.toStdString().c_str(), value.toDouble() );
        else if( type == QVariant::Bool   ) comp->setProperty( propName.toStdString().c_str(), value.toBool() );
        else if( type == QVariant::PointF )
        {
            QStringList coord = value.toString().split(",");
            qreal x = coord.takeFirst().toDouble();
            qreal y = coord.takeFirst().toDouble();
            comp->setProperty( propName.toStdString().c_str(), QPointF( x, y ) );
        }
        else if( type == QVariant::StringList )
        {
            QStringList list= value.toString().split(",");
            comp->setProperty( propName.toStdString().c_str(), list );
        }
        else if//( (lowN=="mem") || (lowN=="eeprom") ) //
            ( type == QVariant::UserType ) // QList<int> or ?? QVector<int>
        {
            QString data = value.toString();
            if( data.isEmpty() ) continue;
            QStringList list = data.split(",");

            QVector<int> vmem;
            for( int x=0; x<list.size(); ++x ) vmem.append( list.at(x).toInt() );

            value = QVariant::fromValue( vmem );
            comp->setProperty( propName.toStdString().c_str(), value );
        }
        else comp->setProperty( propName.toStdString().c_str(), value );
}   }

void Circuit::pasteDomDoc( QDomDocument* doc )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Component*> compList;   // Component List
    QList<Component*> conList;    // Connector List
    QList<Node*>      jointList;  // Joint List

    QHash<QString, eNode*> nodMap;

    m_busy    = true;

    QDomElement circuit = doc->documentElement();
    QDomNode node = circuit.firstChild();

    while( !node.isNull() )
    {
        QDomElement   element = node.toElement();
        const QString tagName = element.tagName();

        if( tagName == "item" )
        {
            QString objNam = element.attribute( "objectName"  ); // Data in simu file
            QString type   = element.attribute( "itemtype"  );
            QString id     = objNam.split("-").first()+"-"+newSceneId(); // Create new id

            element.setAttribute( "objectName", id  );

            if( type == "Connector" )
            {
                Pin* startpin  = 0l;
                Pin* endpin    = 0l;
                QString startpinid    = element.attribute( "startpinid" );
                QString endpinid      = element.attribute( "endpinid" );

                QString startCompName = getCompId( startpinid );
                QString endCompName   = getCompId( endpinid );

                startpinid.replace( startCompName, m_idMap[startCompName] );
                endpinid.replace( endCompName, m_idMap[endCompName] );

                startpin = m_pinMap[startpinid];
                endpin   = m_pinMap[endpinid];
                
                if( !startpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.first().toInt();
                    int itemY = pointList.at(1).toInt();

                    startpin = findPin( itemX, itemY, startpinid );
                }
                if( !endpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.at(pointList.size()-2).toInt();
                    int itemY = pointList.last().toInt();

                    endpin = findPin( itemX, itemY, endpinid );
                }
                if( startpin && startpin->isConnected() ) startpin = 0l;
                if( endpin   && endpin->isConnected() )   endpin   = 0l;

                if( startpin && endpin )    // Create Connector
                {
                    Connector* con  = new Connector( this, type, id, startpin, endpin );

                    element.setAttribute( "startpinid", startpin->objectName() );
                    element.setAttribute(   "endpinid", endpin->objectName() );

                    loadProperties( element, con );

                    QString enodeId = element.attribute( "enodeid" );
                    eNode*  enode   = nodMap[enodeId];
                    if( !enode )                    // Create eNode and add to enodList
                    {
                        enode = new eNode( "Circ_eNode-"+newSceneId() );
                        nodMap[enodeId] = enode;
                    }
                    con->setEnode( enode );

                    QStringList plist = con->pointList();   // add lines to connector
                    int p1x = plist.first().toInt();
                    int p1y = plist.at(1).toInt();
                    int p2x = plist.at(plist.size()-2).toInt();
                    int p2y = plist.last().toInt();

                    con->addConLine( con->x(),con->y(), p1x, p1y, 0 );

                    int count = plist.size();
                    for (int i=2; i<count; i+=2)
                    {
                        p2x = plist.at(i).toInt();
                        p2y = plist.at(i+1).toInt();
                        con->addConLine( p1x, p1y, p2x, p2y, i/2 );
                        p1x = p2x;
                        p1y = p2y;
                    }
                    con->updateConRoute( startpin, startpin->scenePos() );
                    con->updateConRoute( endpin, endpin->scenePos() );
                    con->remNullLines();
                    conList.append( con );
                    startpin->registerPins( enode );
                    endpin->registerPins( enode );
                }
                else // Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null startpin in " << objNam << startpinid;
                    if( !endpin )   qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null endpin in "   << objNam << endpinid;
            }   }
            else if( type == "Node")
            {
                m_idMap[objNam] = id;                              // Map simu id to new id
                
                Node* joint = new Node( this, type, id );
                loadProperties( element, joint );
                compList.append( joint );
                jointList.append( joint );

                if( m_pasting ) joint->setSelected( true );
            }
            else
            {
                if( objNam == "" ) objNam = id;
                m_idMap[objNam] = id;              // Map simu id to new id
                Component* item = createItem( type, id, objNam );
                
                if( item )
                {
                    loadProperties( element, item );
                    compList.append( item );
                    if( m_pasting ) item->setSelected( true );
                }
                else qDebug() << " ERROR Creating Component: "<< type << id;
        }   }
        node = node.nextSibling();
    }
    if( m_pasting )
    {
        for( Component* item : compList ) item->move( m_deltaMove );
        for( Component* item : conList )
        {
            Connector* con = static_cast<Connector*>( item );
            con->setSelected( true );
            con->move( m_deltaMove );
    }   }
    // Take care about unconnected Joints
    for( Node* joint : jointList ) joint->remove(); // Only removed if some missing connector
    
    m_busy = false;
    QApplication::restoreOverrideCursor();
}

void Circuit::circuitToDom()
{
    m_domDoc.clear();
    QDomElement circuit = m_domDoc.createElement("circuit");

    circuit.setAttribute( "type",   "simulide_0.5" );
    circuit.setAttribute( "stepSize", QString::number( m_simulator->stepSize() ) );
    circuit.setAttribute( "stepsPS", QString::number( m_simulator->stepsPerSec() ) );
    circuit.setAttribute( "NLsteps", QString::number( m_simulator->maxNlSteps() ) );
    circuit.setAttribute( "animate", QString::number( animate() ) );
    //circuit.setAttribute( "drawGrid",    QString( drawGrid()?"true":"false"));
    //circuit.setAttribute( "showScroll",  QString( showScroll()?"true":"false"));

    m_domDoc.appendChild(circuit);

    listToDom( &m_domDoc, &m_compList );

    for( Component* comp : m_conList )
    {
        Connector* con = static_cast<Connector*>( comp );
        con->remNullLines();
    }
    listToDom( &m_domDoc, &m_conList );

    circuit.appendChild( m_domDoc.createTextNode( "\n \n" ) );
}

void Circuit::listToDom( QDomDocument* doc, QList<Component*>* complist )
{
    for( Component* comp : *complist )
    {
        if( comp->isHidden() ) continue;

        bool isNumber = false;  // Don't save internal items
        comp->objectName().split("-").last().toInt( &isNumber );

        if( isNumber )
        {
            QDomElement root = doc->firstChild().toElement();
            QDomElement elm = doc->createElement("item");

            objectToDom( &elm, comp );
            if( comp->itemType() == "Subcircuit" )
            {
                SubCircuit* subc = (SubCircuit*)comp;
                Component* mainComp = subc->getMainComp();
                if( mainComp )                            // Save MainComp Properties.
                {
                    QDomElement mainElm = doc->createElement("mainCompProps");
                    objectToDom( &mainElm, mainComp, true );
                    elm.appendChild( mainElm );
            }   }
            QDomText objId = m_domDoc.createTextNode(  "\n\nUnique  Id: "+comp->objectName()
                                                      +": \nCircuit Id: "+comp->idLabel()+"\n" );
            root.appendChild( objId );
            root.appendChild( elm );
}   }   }

void Circuit::objectToDom( QDomElement* elm, Component* comp, bool onlyMain )
{
    const QMetaObject* metaobject = comp->metaObject();

    int count = metaobject->propertyCount();
    for( int i=0; i<count; i++ )
    {
        QMetaProperty metaproperty = metaobject->property(i);
        const char* name = metaproperty.name();
        if( onlyMain ) // Main Component in Subcircuit: don't load basic properties.
        {
            if( !metaproperty.isUser() )        continue;
            if( !metaproperty.isDesignable() )  continue;
            if( strcmp(name, "itemtype") == 0 ) continue;
            if( strcmp(name, "Show_id") == 0 )  continue;
            if( strcmp(name, "id") == 0 )       continue;
        }

        QVariant value = comp->property( name );
        QVariant::Type type = metaproperty.type();

        if( type == QVariant::StringList )
        {
            QStringList list= value.toStringList();
            elm->setAttribute( name, list.join(",") );
        }
        else if( type == QVariant::PointF )
        {
            QPointF point = value.toPointF();
            elm->setAttribute( name, QString::number(point.x())+","+QString::number(point.y())  );
        }
        else if//  ( (QString(name)=="Mem") || (QString(name)=="eeprom") )
            ( type == QVariant::UserType ) // QList<int> or ?? QVector<int>
        {
            QVector<int> vmem = value.value<QVector<int>>();

            QStringList list;
            for( int val : vmem ) list << QString::number( val );

            elm->setAttribute( name, list.join(",") );
        }
        else elm->setAttribute( name, value.toString() );
}   }

bool Circuit::saveDom( QString &fileName, QDomDocument* doc )
{
    QFile file( fileName );

    if( !file.open( QFile::WriteOnly | QFile::Text ))
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(0l, "Circuit::saveCircuit",
        tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc->toString();
    file.close();

    return true;
}

bool Circuit::saveCircuit( QString fileName )
{
    if( m_conStarted ) return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if( !fileName.endsWith(".simu") ) fileName.append(".simu");

    QString oldFilePath = m_filePath;
    m_filePath = fileName;

    circuitToDom();
    bool saved = saveDom( fileName, &m_domDoc );

    if( saved )
    {
        if( !m_backupPath.isEmpty() )
        {
            QFile::remove( m_backupPath ); // remove backup file
            m_backupPath = "";
    }   }
    else m_filePath = oldFilePath;

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
                                          tr("Circuits (*.simu);;All files (*.*)"));

    if( !fileName.isEmpty() && fileName.endsWith(".simu") )
        loadCircuit( fileName );

    m_pasting = false;
}

Component* Circuit::createItem( QString type, QString id, QString objNam )
{
    for( LibraryItem* libItem : ItemLibrary::self()->items() )
    {
        if( !(libItem->type()==type) ) continue;

        Component* comp = libItem->createItemFnPtr()( this, type, id );

        if( comp )
        {
            QString category = libItem->category();
            if( ( category != "Meters" )
            &&  ( category != "Sources" )
            &&  ( category != "Other" ) )
                comp->setPrintable( true );
        }
        return comp;
    }
    return 0l;
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
        Component* comp = qgraphicsitem_cast<Component* >( item );
        if( comp )
        {
            if( (comp->itemType()!="Node")
             && (comp->itemType()!="Connector")
             && !components.contains( comp ) )
                components.append( comp );
        }
        else
        {
            ConnectorLine* line = qgraphicsitem_cast<ConnectorLine* >( item );
            if( line->objectName() == "" )
            {
                Connector* con = line->connector();
                if( !connectors.contains( con ) ) connectors.append( con );
    }   }   }
    //if( !connectors.isEmpty() ) if( m_simulator->isRunning() )
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
        removeItem( comp );
        m_compList.removeOne( comp );
    }
    delete comp; // crash in recent versions bcos already removed in removeItem( comp );
}

void Circuit::remove() // Remove everything ( Clear Circuit )
{
    if( m_conStarted ) return;
    m_busy = true;

    while( !m_compList.isEmpty() )
    {
        Component* comp = m_compList.takeFirst();

        bool isNumber = false;               // Don't remove internal items

        comp->objectName().split("-").last().toInt( &isNumber ); // TODO: Find a proper way !!!!!!!!!!!

        if( isNumber && !(comp->itemType()=="Node") )
        {
            if( comp->scene() == this ) removeItem( comp );
            removeComp( comp );
        }
    }
    m_busy = false;
}

void Circuit::deselectAll()
{
    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
}

void Circuit::saveState()
{
    if( m_conStarted ) return;

    m_changed = true;

    if( m_busy || m_deleting ) return;
    m_deleting = true;

    for( QDomDocument* doc : m_redoStack ) delete doc;

    m_redoStack.clear();
    circuitToDom();
    m_undoStack.append( new QDomDocument() );
    m_undoStack.last()->setContent( m_domDoc.toString() );

    QString title = MainWindow::self()->windowTitle();
    if( !title.endsWith('*') ) MainWindow::self()->setWindowTitle(title+'*');

    m_deleting = false;
}

void Circuit::saveChanges()
{
    if( m_simulator->isRunning() ) return;
    if( !m_changed || m_conStarted || m_busy ) return;
    m_changed = false;

    circuitToDom();
    m_backupPath = m_filePath;

    QFileInfo bckDir( m_backupPath );

    if( !bckDir.isWritable() )
        m_backupPath = SIMUAPI_AppPath::self()->RWDataFolder().absolutePath()+"/_backup.simu";

    if( !m_backupPath.endsWith( "_backup.simu" ))
        m_backupPath.replace( ".simu", "_backup.simu" );

    if( saveDom( m_backupPath, &m_domDoc ) )
        MainWindow::self()->settings()->setValue( "backupPath", m_backupPath );
}

void Circuit::undo()
{
    if( m_conStarted || m_undoStack.isEmpty() ) return;

    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    circuitToDom();
    m_redoStack.prepend( new QDomDocument() );
    m_redoStack.first()->setContent( m_domDoc.toString() );

    remove();
    QDomDocument* doc = m_undoStack.takeLast();
    m_domDoc.setContent( doc->toString());

    m_seqNumber = 0;
    loadDomDoc( &m_domDoc );
}

void Circuit::redo()
{
    if( m_conStarted || m_redoStack.isEmpty()) return;

    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();

    circuitToDom();
    m_undoStack.append( new QDomDocument() );
    m_undoStack.last()->setContent( m_domDoc.toString() );

    remove();
    QDomDocument* doc = m_redoStack.takeFirst();
    m_domDoc.setContent( doc->toString());

    m_seqNumber = 0;
    loadDomDoc( &m_domDoc );
}

void Circuit::copy( QPointF eventpoint )
{
    if( m_conStarted ) return;

    m_eventpoint = togrid(eventpoint);

    QList<Component*> complist;
    QList<QGraphicsItem*> itemlist = selectedItems();

    for( QGraphicsItem* item : itemlist )
    {
        Component* comp =  qgraphicsitem_cast<Component*>( item );
        if( comp )
        {
            if( comp->itemType() == "Connector" )
            {
                Connector* con = static_cast<Connector*>( comp );
                con->remNullLines();

                complist.append( con );
            }
            else complist.prepend( comp );
    }   }
    m_copyDoc.clear();
    QDomElement root = m_copyDoc.createElement("circuit");
    root.setAttribute( "type", "simulide_0.4" );
    m_copyDoc.appendChild(root);

    listToDom( &m_copyDoc, &complist );

    QString px = QString::number( m_eventpoint.x() );
    QString py = QString::number( m_eventpoint.y() );
    QString clipTextText = px+","+py+"eventpoint"+m_copyDoc.toString();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText( clipTextText );
}

void Circuit::paste( QPointF eventpoint )
{
    if( m_conStarted ) return;

    QClipboard *clipboard = QApplication::clipboard();
    QString clipText = clipboard->text();
    if( !clipText.contains( "eventpoint") ) return;

    if( m_simulator->isRunning() ) CircuitWidget::self()->powerCircOff();
    
    bool animate = m_animate;
    saveState();
    m_pasting = true;
    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    QStringList clipData = clipText.split( "eventpoint" );
    clipText = clipData.last();
    m_copyDoc.setContent( clipText );

    clipData = clipData.first().split(",");
    int px = clipData.first().toInt();
    int py = clipData.last().toInt();
    m_eventpoint = QPointF( px, py );

    m_deltaMove = togrid(eventpoint) - m_eventpoint;

    pasteDomDoc( &m_copyDoc );

    m_pasting = false;
    setAnimate( animate );
}

void Circuit::newconnector( Pin*  startpin )
{
    saveState();
    m_conStarted = true;

    QString type = QString("Connector");
    QString id = type;
    id.append( "-" );
    id.append( newSceneId() );

    new_connector = new Connector( this, type, id, startpin );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();

    new_connector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );

    addItem(new_connector);
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
{
    for( Component* comp : m_conList )
    {
        Connector* con = static_cast<Connector*>( comp );
        con->updateLines();
}   }

void Circuit::bom()
{
    if( m_conStarted ) return;

    QString fileName = m_filePath;
    fileName.replace( fileName.lastIndexOf( ".simu" ), 5, "-bom.txt" );

    fileName = QFileDialog::getSaveFileName( MainWindow::self()
                            , tr( "Bill Of Materials" )
                            , fileName
                            , "(*.*)"  );

    if( fileName.isEmpty() ) return;

    QStringList bom;

    for( Component* comp : m_compList )
    {
        bool isNumber = false;
        comp->objectName().split("-").last().toInt( &isNumber );

        if( isNumber ) bom.append( comp->print() );
    }
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
        else if( key == Qt::Key_V )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());
            paste( m_graphicView->mapToScene( p ) );
        }
        else if( key == Qt::Key_Z ) undo();
        else if( key == Qt::Key_Y ) redo();
        else if( key == Qt::Key_N ) CircuitWidget::self()->newCircuit();
        else if( key == Qt::Key_S )
        {
            if( event->modifiers() & Qt::ShiftModifier)
                 CircuitWidget::self()->saveCircAs();
            else CircuitWidget::self()->saveCirc();
        }
        else if( key == Qt::Key_O ) CircuitWidget::self()->openCirc();
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
            //qDebug() << "Circuit::keyPressEven" << keys;
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
    QString type = event->mimeData()->html();
    QString id   = event->mimeData()->text();

    QString file = "file://";
    if( id.startsWith( file ) )
    {
        id.replace( file, "" ).replace("\r\n", "" );
#ifdef _WIN32
        if( id.startsWith( "/" )) id.remove( 0, 1 );
#endif
        QString loId = id.toLower();

        if( loId.endsWith( ".jpg")
         || loId.endsWith( ".png")
         || loId.endsWith( ".gif"))
        {
            file = id;
            type = "Image";
            id   = "Image";
            Component* enterItem = createItem( type, id+"-"+newSceneId() );
            if( enterItem )
            {
                saveState();
                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( CircuitView::self()->mapToScene( cPos ) );
                enterItem->setBackground( file );
                addItem( enterItem );
        }   }
        else CircuitWidget::self()->loadCirc( id );
}   }

void Circuit::drawBackground ( QPainter*  painter, const QRectF & rect )
{
    Q_UNUSED( rect );
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

    for( int i = 4; i<endx; i+=8 )
    {
        painter->drawLine( i, starty, i, endy );
        painter->drawLine(-i, starty,-i, endy );
    }
    for( int i = 4; i<endy; i+=8 )
    {
        painter->drawLine( startx, i, endx, i);
        painter->drawLine( startx,-i, endx,-i);
}   }

void Circuit::updatePin( ePin* epin, QString newId )
{
    QString pinId = newId;
    Pin* pin = static_cast<Pin*>( epin );
    addPin( pin, pinId );
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
    if( show )
    {
        m_graphicView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        m_graphicView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "true" );
    }
    else
    {
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
