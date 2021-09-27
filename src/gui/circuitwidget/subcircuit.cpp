/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include <QDomDocument>

#include "subcircuit.h"
#include "itemlibrary.h"
#include "componentselector.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "tunnel.h"
#include "node.h"
#include "e-node.h"
#include "utils.h"
#include "simuapi_apppath.h"

#include "stringprop.h"
#include "boolprop.h"

Component* SubCircuit::construct( QObject* parent, QString type, QString id )
{
    SubCircuit* subcircuit = new SubCircuit( parent, type,  id );
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subcircuit );
        subcircuit->deleteLater();
        subcircuit = 0l;
        m_error = 0;
    }
    return subcircuit;
}

LibraryItem* SubCircuit::libraryItem()
{
    return new LibraryItem(
        tr("Subcircuit"),
        tr(""),         // Not dispalyed
        "",
        "Subcircuit",
        SubCircuit::construct );
}

SubCircuit::SubCircuit( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    m_name = m_id.split("-").first(); // for example: "atmega328-1" to: "atmega328"
    m_label.setPlainText( m_name );

    m_icColor = QColor( 20, 30, 60 );
    m_attached = false;
    m_boardId = "";
    m_board = NULL;
    m_shield = NULL;
    m_mainComponent = NULL;

    QString dataFile = ComponentSelector::self()->getXmlFile( m_name );

    if( dataFile == "" ){
          MessageBoxNB( "SubCircuit::SubCircuit", "                               \n"+
                    tr( "There are no data files for " )+m_name+"    ");
          m_error = 23;
          return;
    }
    QDomDocument domDoc = fileToDomDoc( dataFile, "SubCircuit::SubCircuit" );
    if( domDoc.isNull() ) { m_error = 1; return; }

    QDomElement   root  = domDoc.documentElement();
    QDomNode      rNode = root.firstChild();

    while( !rNode.isNull() )
    {
        QDomElement itemSet = rNode.toElement();
        QDomNode    node    = itemSet.firstChild();

        QString folder = "";
        if( itemSet.hasAttribute( "folder") )
            folder = itemSet.attribute( "folder" );

        while( !node.isNull() )         // Find the "package", for example 628A is package: 627A, Same pins
        {
            QDomElement element = node.toElement();

            if( element.attribute("name") == m_name )
            {
                QDir dataDir( dataFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name

                QString subcFile = "";
                QString path = "";

                if( folder != "" )
                {
                    path = folder+"/"+m_name+"/"+m_name;
                    m_pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".sim1" );
                }
                if( element.hasAttribute( "folder") )
                {
                    path = element.attribute( "folder" )+"/"+m_name+"/"+m_name;
                    m_pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".sim1" );
                }

                if( element.hasAttribute( "package") )
                    m_pkgeFile = dataDir.filePath( element.attribute( "package" ) );

                if( !m_pkgeFile.endsWith( ".package" ) ) m_pkgeFile += ".package" ;
                
                if( element.hasAttribute( "subcircuit") )
                {
                    subcFile = dataDir.filePath( element.attribute( "subcircuit" ) );
                    if( subcFile.endsWith( ".simu" ) ) subcFile= changeExt( subcFile, ".sim1" );
                    if( !subcFile.endsWith( ".sim1" )) subcFile += ".sim1" ;
                }
                if( !QFile::exists( subcFile) ) subcFile = changeExt( subcFile, ".simu" );

                loadSubCircuit( subcFile );

                if( m_mainComponent )  // Example MCU in subcircuit needs to know where subcircuit is.
                {
                    dataDir.setPath( subcFile );
                    dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                    m_mainComponent->setSubcDir( dataDir.absolutePath() );
                    m_mainComponent->m_subcircuit = this;
                }
                break;
            }
            node = node.nextSibling();
        }
        rNode = rNode.nextSibling();
    }
    if( m_error != 0 )
    {
        for( Component* comp : m_compList )
        {
            comp->setParentItem( NULL );
            Circuit::self()->removeComp( comp );
    }   }
    else    initChip();

    addPropGroup( { tr("Main"), {
new BoolProp<SubCircuit>( "Logic_Symbol", tr("Logic Symbol"),"", this, &SubCircuit::logicSymbol, &SubCircuit::setLogicSymbol ),
    }} );
    addPropGroup( { tr("Hidden"), {
new StringProp<SubCircuit>( "BoardId" , "","", this, &SubCircuit::boardId, &SubCircuit::setBoardId )
    }} );
}
SubCircuit::~SubCircuit(){}

void SubCircuit::loadSubCircuit( QString fileName )
{
    QFile file( fileName );

    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
        MessageBoxNB( "SubCircuit::loadSubCircuit", "                               \n"+
                  tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        m_error = 32;
        return;
    }
    QDomDocument domDoc;

    if( !domDoc.setContent(&file) )
    {
        MessageBoxNB( "SubCircuit::loadSubCircuit", "                               \n"+
                  tr("Cannot set file %1\nto DomDocument").arg(fileName) );
        file.close();
        m_error = 33;
        return;
    }
    file.close();

    QStringList graphProps;
    for( propGroup pg : m_properties ) // Create list of "Graphical" poperties (We don't need them)
    {
        if( (pg.name != "CompGraphic") ) continue;
        for( ComProperty* prop : pg.propList ) graphProps.append( prop->name() );
        break;
    }

    Circuit* circ = Circuit::self();
    QHash<QString, eNode*> nodMap;
    QDomNode node = domDoc.documentElement().firstChild();

    while( !node.isNull() )
    {
        QDomElement   element = node.toElement();
        const QString tagName = element.tagName();

        if( tagName == "item" )
        {
            QString objNam = element.attribute( "objectName"  ); // Data in simu file
            QString type   = element.attribute( "itemtype"  );
            QString id     = objNam;
            id = id.remove( id.lastIndexOf("-"), 100 )+"-"+circ->newSceneId(); // Create new id

            element.setAttribute( "objectName", id  );

            if( type == "Connector" )
            {
                QString startPinId = element.attribute( "startpinid" );
                QString endPinId   = element.attribute( "endpinid" );
                Pin* startPin  = getConPin( element.attribute( "startpinid" ) );
                Pin* endPin    = getConPin( element.attribute( "endpinid" ) );

                if( startPin && endPin )    // Create Connector
                {
                    /*Connector* con  = new Connector( this, type, id, startPin, endPin, false );

                    element.setAttribute( "startpinid", startPin->objectName() );
                    element.setAttribute(   "endpinid", endPin->objectName() );

                    circ->loadProperties( &element, con );*/

                    QString enodeId = element.attribute( "enodeid" );
                    eNode*  enode   = nodMap[enodeId];

                    if( !enode )          // Create eNode and add to enodList
                    {
                        enode = new eNode( "Circ_eNode-"+circ->newSceneId() );
                        nodMap[enodeId] = enode;
                    }
                    //con->setEnode( enode );
                    if( startPin->isBus() ) enode->setIsBus( true );
                    startPin->setConPin( endPin );
                    endPin->setConPin( startPin );
                    startPin->registerPins( enode );
                    endPin->registerPins( enode );
                }
                else // Start or End pin not found
                {
                    if( !startPin ) qDebug() << "\n   ERROR!!  SubCircuit::loadDomDoc: "+m_id+" null startPin in " << objNam<<startPinId;
                    if( !endPin )   qDebug() << "\n   ERROR!!  SubCircuit::loadDomDoc: "+m_id+" null endPin in "   << objNam<<endPinId;
            }   }
            else if( type == "Package" ) { ; }
            else{
                Component* comp = NULL;
                if( objNam == "" ) objNam = id;
                if( type == "Node" ) comp = new Node( this, type, id );
                else                 comp = circ->createItem( type, id );
                circ->m_idMap[objNam] = id; // Map simu id to new id

                if( comp )
                {
                    QDomNamedNodeMap atrs = element.attributes();
                    for( int i=0; i<atrs.length(); ++i )   // Load Properties
                    {
                        QString propName = atrs.item(i).nodeName();
                        if( graphProps.contains( propName )) continue; // Don't load "Component" properties.

                        QString value = element.attribute( propName );
                        if( !comp->setProperty( propName, value ) )
                            qDebug() << "SubCircuit"<<m_id<<" Wrong Property: "<<comp->getUid()<<propName<<value;
                    }
                    ///circ->loadProperties( &element, comp );

                    comp->setParentItem( this );

                    if( comp->isGraphical() )
                    {
                        QPointF pos = comp->boardPos();
                        if( pos == QPointF( -1e6, -1e6 ) ) pos = QPointF( 0, 0 );
                        comp->moveTo( pos );
                        comp->setRotation( comp->boardRot() );
                    }
                    else comp->moveTo( QPointF(20, 20) );

                    if( comp->isMainComp() ) m_mainComponent = comp; // This component will add it's Context Menu

                    comp->setHidden( true, true );
                    circ->compList()->removeOne( comp );
                    m_compList.append( comp );

                    if( type == "Tunnel" ) // Make Tunnel names unique for this subcircuit
                    {
                        Tunnel* tunnel = static_cast<Tunnel*>( comp );
                        tunnel->setUid( tunnel->name() );
                        tunnel->setName( m_id+"-"+tunnel->name() );
                        m_subcTunnels.append( tunnel );
                }   }
                else qDebug() << " ERROR Creating Component: "<< type << id;
        }   }
        node = node.nextSibling();
}   }

Pin* SubCircuit::getConPin( QString pinId )
{
    Pin* pin = 0l;
    QString compName;
    if( pinId.contains("Seg"))
    {
        compName = pinId;
        compName = compName.remove( compName.lastIndexOf("-"), 100 );
    }
    else                       compName = Circuit::self()->getCompId( pinId );
    QString newName  = Circuit::self()->m_idMap.value( compName );

    if( !newName.isEmpty() ) pinId.replace( compName, newName );
    pin = Circuit::self()->m_pinMap[pinId];
    if( pin && pin->isConnected() ) pin = NULL;

    return pin;
}

void SubCircuit::addPin(QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length  )
{
    if( m_initialized && m_pinTunnels.contains( m_id+"-"+id ) )
    {
        updatePin( id, type, label, pos, xpos, ypos, angle, length );
    }else{
        QColor color = Qt::black;
        if( !m_isLS ) color = QColor( 250, 250, 200 );

        Tunnel* tunnel = new Tunnel( this, "Tunnel", m_id+"-"+id );
        Circuit::self()->compList()->removeOne( tunnel );
        m_compList.append( tunnel );

        QString pId = m_id+"-"+id;
        tunnel->setParentItem( this );
        tunnel->setAcceptedMouseButtons( Qt::NoButton );
        tunnel->setShowId( false );
        tunnel->setName( pId ); // Make tunel name unique for this component
        tunnel->setPos( xpos, ypos );
        tunnel->setPacked( true );
        m_pinTunnels.insert( pId, tunnel );

        Pin* pin = tunnel->getPin();
        pin->setObjectName( m_id+"-"+id );
        pin->setId( m_id+"-"+id );
        pin->setLabelColor( color );
        pin->setLabelText( label );
        connect( this, SIGNAL( moved() ), pin, SLOT( isMoved() ), Qt::UniqueConnection );

        if     ( type == "inverted" ) pin->setInverted( true );
        else if( type == "unused" )   pin->setUnused( true );
        else if( type == "null" )
        {
            pin->setVisible( false );
            pin->setLabelText( "" );
        }
        if     ( angle == 90 )  tunnel->setRotation( -90 ); // QGraphicsItem 0º i at right side
        else if( angle >= 180 ) tunnel->setRotated( true ); // Our Pins at left side
        if( angle == 270 ) tunnel->setRotation( angle );

        pin->setLength( length );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );

        m_ePin[pos-1] = pin;
}   }

void SubCircuit::updatePin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length )
{
    Pin* pin = NULL;
    Tunnel* tunnel = m_pinTunnels.value( m_id+"-"+id );
    tunnel->setPos( xpos, ypos );
    tunnel->setRotated( false );
    tunnel->setRotation( angle );
    if( angle >= 180 ) tunnel->setRotated( true );
    if( angle == 180)  tunnel->setRotation( 0 );

    pin = tunnel->getPin();
    if( !pin )
    {
        qDebug() <<"SubCircuit::updatePin Pin Not Found:"<<id<<type<<label;
        return;
    }
    pin->setLabelText( label );
    pin->setLabelPos();
    pin->setLength( length );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );

    type = type.toLower();

    if( m_isLS )
    {
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        if( (type=="unused") || (type=="nc") ) type = "null";
    }
    else pin->setLabelColor( QColor( 250, 250, 200 ) );

    pin->setUnused( (type=="unused") || (type=="nc") );
    pin->setInverted( type == "inverted" );

    if( type == "null" )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
    }
    else pin->setVisible( true );

    pin->isMoved();
}

void SubCircuit::setLogicSymbol( bool ls )
{
    if( !m_initialized ) return;
    if( m_isLS == ls ) return;
    Chip::setLogicSymbol( ls );

    if( m_isLS )
    {
        for( QString tNam : m_pinTunnels.keys() )
        {
            Tunnel* tunnel = m_pinTunnels.value( tNam );
            Pin* pin = tunnel->getPin();
            if( pin->unused() )
            {
                pin->setVisible( false );
                pin->setLabelText( "" );
}   }   }   }

void SubCircuit::remove()
{
    if( m_board ) return;

    for( Component* comp : m_compList )
    {
        if( comp->itemType()=="Node" ) continue;
        comp->setParentItem( NULL );
        Circuit::self()->removeComp( comp );
    }
    if( m_shield ) // there is a shield attached to this
    {
        m_shield->setParentItem( NULL );
        m_shield->setBoard( NULL );
        Circuit::self()->removeComp( m_shield );
    }
    Component::remove();
}

void SubCircuit::slotAttach()
{
    QList<QGraphicsItem*> list = this->collidingItems();
    for( QGraphicsItem* it : list )
    {
        if( it->type() == 65536+1 )    // Component found
        {
            Component* comp =  qgraphicsitem_cast<Component*>( it );
            if( comp->itemType() == "Subcircuit" )
            {
                SubCircuit* board =  (SubCircuit*)comp;
                if( !(board->subcType() == Board) ) continue;

                if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
                Circuit::self()->saveState();

                m_board = board;
                m_boardId = m_board->m_id;
                m_board->setShield( this );

                m_circPos = this->pos();

                int origX = 8*(m_board->pkgWidth()-m_width)/2;
                this->setParentItem( m_board );
                this->moveTo( QPointF(origX, 0) );
                this->setRotation(0);

                for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->uid() );
                m_attached = true;
                break;
}   }   }   }

void SubCircuit::slotDetach()
{
    if( m_board )
    {
        if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
        Circuit::self()->saveState();

        m_board->setShield( NULL );
        this->moveTo( m_circPos );
        this->setParentItem( NULL );
        for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_id+"-"+tunnel->uid() );
        m_board = NULL;
    }
    m_attached = false;
}

void SubCircuit::connectBoard()
{
    QString name = Circuit::self()->origId( m_boardId );
    if( name != "" ) m_boardId = name;

    Component* comp = Circuit::self()->getCompById( m_boardId );
    if( comp && comp->itemType() == "Subcircuit" )
    {
        Circuit::self()->compList()->removeOne( this );

        m_board = static_cast<SubCircuit*>(comp);

        m_board->setShield( this );
        this->setParentItem( m_board );
        for( Tunnel* tunnel : m_subcTunnels ) tunnel->setName( m_boardId+"-"+tunnel->uid() );
        m_attached = true;
}   }

void SubCircuit::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else{
        event->accept();
        QMenu* menu = new QMenu();
        Component* mainComp = m_mainComponent;
        QString id = m_id;

        if( m_subcType == Shield )
        {
            if( m_attached )
            {
                QAction* detachAction = menu->addAction(QIcon(":/detach.png"),tr("Detach") );
                connect( detachAction, SIGNAL( triggered()), this, SLOT(slotDetach()) );
            }else{
                QAction* attachAction = menu->addAction(QIcon(":/attach.png"),tr("Attach") );
                connect( attachAction, SIGNAL( triggered()), this, SLOT(slotAttach()) );
            }
            menu->addSection( "" );
            if( m_board && m_board->m_mainComponent )
            {
                mainComp = m_board->m_mainComponent;
                id = "Board "+m_board->m_id;
        }   }
        if( mainComp )
        {
            menu->addSection( "                            " );
            menu->addSection( mainComp->itemType()+" at "+id );
            menu->addSection( "" );
            mainComp->contextMenu( NULL, menu );

            menu->addSection( "                            " );
            menu->addSection( id );
            menu->addSection( "" );
        }
        if( m_board ) m_board->contextMenu( event, menu );
        else          Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

QString SubCircuit::toString()
{
    QString item = CompBase::toString();
    if( !m_mainComponent ) return item;

    item.remove( "/>\n" );
    item += ">\n";
    item += m_mainComponent->toString().replace( "<item ", "<mainCompProps ");
    item += "</item>\n";

    return item;
}

#include "moc_subcircuit.cpp"
