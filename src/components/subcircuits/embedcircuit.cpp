/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QMenu>
#include <QDebug>

#include "embedcircuit.h"
#include "component.h"
#include "comproperty.h"
#include "circuit.h"
#include "tunnel.h"
#include "node.h"
#include "utils.h"
#include "mcu.h"

QString EmbedCircuit::m_subcDir = "";
QStringList EmbedCircuit::s_graphProps;

EmbedCircuit::EmbedCircuit( QString name, QString id, Chip *component)
{
    m_component = component;
    m_ecName = name;
    m_ecId = id;

    if( s_graphProps.isEmpty() ) loadGraphProps();
}
EmbedCircuit::~EmbedCircuit(){}

void EmbedCircuit::loadSubCircuitFile( QString file )
{
    QString doc = fileToString( file, "SubCircuit::loadSubCircuit" );

    QString oldFilePath = Circuit::self()->getFilePath();
    Circuit::self()->setFilePath( file );             // Path to find subcircuits/Scripted in our data folder

    loadSubCircuit( doc );

    Circuit::self()->setFilePath( oldFilePath ); // Restore original filePath
}

void EmbedCircuit::loadSubCircuit( QString doc )
{
    QString numId = m_ecId;
    numId = numId.split("-").last();
    Circuit* circ = Circuit::self();

    QList<Linker*> linkList;   // Linked  Component list

    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( !line.startsWith("<item") ) continue;

        QVector<propStr_t> properties = parseXmlProps( line );

        propStr_t itemType = properties.takeFirst();
        if( itemType.name != "itemtype") continue;
        QString type = itemType.value.toString();

        if( type == "Package" || type == "Subcircuit" ) continue;

        if( type == "Connector" )
        {
            QString startPinId, endPinId, enodeId;
            QStringList pointList;

            for( propStr_t prop : properties )
            {
                if     ( prop.name == "startpinid") startPinId = numId+"@"+prop.value.toString();
                else if( prop.name == "endpinid"  ) endPinId   = numId+"@"+prop.value.toString();
                else if( prop.name == "pointList" ) pointList  = prop.value.toString().split(",");
            }

            Pin* startPin = circ->m_LdPinMap.value( startPinId );
            Pin* endPin   = circ->m_LdPinMap.value( endPinId );

            if( !startPin ) startPin = findPin( startPinId );
            if( !endPin   ) endPin   = findPin( endPinId );

            if( startPin && endPin ) // Create Connection
            {
                startPin->setConPin( endPin );
                endPin->setConPin( startPin );
            }
            else // Start or End pin not found
            {
                if( !startPin ) qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_ecName<<m_ecId+" null startPin in "<<type<<startPinId;
                if( !endPin )   qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_ecName<<m_ecId+" null endPin in "  <<type<<endPinId;
        }   }
        else{
            Component* comp = nullptr;

            propStr_t circId = properties.takeFirst();
            if( circId.name != "CircId") continue; /// ERROR
            QString uid = circId.value.toString();
            QString newUid = numId+"@"+uid;

            if( type == "Node" ) comp = new Node( type, newUid );
            else                 comp = circ->createItem( type, newUid, false );

            if( !comp ){
                qDebug() << "SubCircuit:"<<m_ecName<<m_ecId<< "ERROR Creating Component: "<<type<<uid;
                continue;
            }
            comp->setIdLabel( uid ); // Avoid parent Uids in label

            Mcu* mcu = nullptr;
            if( comp->itemType() == "MCU" )
            {
                comp->remProperty("Logic_Symbol");
                mcu = (Mcu*)comp;
                mcu->m_subcFolder = m_subcDir+"/";
            }

            for( propStr_t prop : properties )
            {
                QString propName = prop.name.toString();
                if( !s_graphProps.contains( propName ) ) comp->setPropStr( propName, prop.value.toString() );
            }
            if( mcu ) mcu->m_subcFolder = "";

            comp->setup();
            comp->setParentItem( m_component );

            if( m_component->isBoard() && comp->isGraphical() )
            {
                QPointF pos = comp->boardPos();

                comp->moveTo( pos );
                comp->setRotation( comp->boardRot() );
                comp->setHflip( comp->boardHflip() );
                comp->setVflip( comp->boardVflip() );

                if( !m_component->collidesWithItem( comp ) ) // Don't show Components out of Board
                {
                    comp->setBoardPos( QPointF(-1e6,-1e6 ) ); // Used in setLogicSymbol to identify Components not visible
                    comp->moveTo( QPointF( 0, 0 ) );
                    comp->setVisible( false );
                }
                comp->setHidden( true, true, true ); // Boards: hide non graphical
                if( m_component->m_isLS && m_component->m_packageList.size() > 1 ) comp->setVisible( false ); // Don't show any component if Logic Symbol
            }
            else{
                comp->moveTo( QPointF(20, 20) );
                comp->setVisible( false );     // Not Boards: Don't show any component
            }

            if( comp->isMainComp() ) m_mainComponents[uid] = comp; // This component will add it's Context Menu and properties

            m_compList.append( comp );

            if( comp->m_isLinker ){
                Linker* l = dynamic_cast<Linker*>(comp);
                if( l->hasLinks() ) linkList.append( l );
            }

            if( type == "Tunnel" ) // Make Circuit Tunnel names unique for this subcircuit
            {
                Tunnel* tunnel = static_cast<Tunnel*>( comp );
                tunnel->setTunnelUid( tunnel->name() );
                tunnel->setName( m_ecId+"-"+tunnel->name() );
                m_subcTunnels.append( tunnel );
            }
    }   }
    for( Linker* l : linkList ) l->createLinks( &m_compList );
}
void EmbedCircuit::addMainCompsMenu( QMenu* menu )
{
    for( Component* mainComp : m_mainComponents.values() )
    {
        QString name = mainComp->idLabel();
        QMenu* submenu = menu->addMenu( QIcon(":/subc.png"), name );
        mainComp->contextMenu( nullptr, submenu );
    }
    menu->addSeparator();
}

Pin* EmbedCircuit::addPin( QString id, QString type, QString label, int, int xpos, int ypos, int angle, int length, int space )
{
    QColor color = Qt::black;
    if( !m_component->m_isLS ) color = QColor( 250, 250, 200 );

    QString pId = m_ecId+"-"+id;
    Tunnel* tunnel = new Tunnel("Tunnel", pId );
    m_compList.append( tunnel );

    tunnel->setParentItem( m_component );
    tunnel->setAcceptedMouseButtons( Qt::NoButton );
    tunnel->setShowId( false );
    tunnel->setTunnelUid( id );
    tunnel->setName( pId );           // Make Pin Tunel names unique for this component
    tunnel->setPos( xpos, ypos );
    tunnel->setPacked( true );
    if( type == "bus" ) tunnel->setIsbus( true );
    m_pinTunnels.insert( pId, tunnel );

    Pin* pin = tunnel->getPin();
    pin->setId( pId );
    pin->setInverted( type == "inverted" || type == "inv" );
    m_component->addSignalPin( pin );

    tunnel->setRotated( angle >= 180 );      // Our Pins at left side
    if     ( angle == 180) tunnel->setRotation( 0 );
    else if( angle == 90 ) tunnel->setRotation(-90 ); // QGraphicsItem 0º i at right side
    else                   tunnel->setRotation( angle );

    pin->setLength( length );
    pin->setSpace( space );
    pin->setLabelColor( color );
    pin->setLabelText( label );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    return pin;
}

Pin* EmbedCircuit::updatePin( QString id, QString type, QString label, int xpos, int ypos, int angle, int length, int space )
{
    Pin* pin = nullptr;
    Tunnel* tunnel = m_pinTunnels.value( m_ecId+"-"+id );
    if( !tunnel ){
        //qDebug() <<"SubCircuit::updatePin Pin Not Found:"<<id<<type<<label;
        return nullptr;
    }
    tunnel->setPos( xpos, ypos );
    tunnel->setRotated( angle >= 180 );      // Our Pins at left side
    tunnel->setIsbus( type == "bus" );

    if     ( angle == 180) tunnel->setRotation( 0 );
    else if( angle == 90 ) tunnel->setRotation(-90 ); // QGraphicsItem 0º i at right side
    else                   tunnel->setRotation( angle );

    pin  = tunnel->getPin();
    type = type.toLower();

    bool unused = type == "unused" || type == "nc";
    pin->setUnused( unused );
    if( unused && m_component->m_isLS )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
        return pin;
    }
    if( m_component->m_isLS ) pin->setLabelColor( QColor( 0, 0, 0 ) );
    else                      pin->setLabelColor( QColor( 250, 250, 200 ) );

    pin->setInverted( type == "inverted" || type == "inv" );
    pin->setLength( length );
    pin->setSpace( space );
    pin->setLabelText( label );
    pin->setVisible( true );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );
    pin->isMoved();

    return pin;
}

Pin* EmbedCircuit::findPin( QString pinId )
{
    QStringList words = pinId.split("-");
    pinId = words.takeLast();
    QString compId = words.join("-");

    for( Component* comp : m_compList ) if( comp->getUid() == compId ) return comp->getPin( pinId );

    return nullptr;
}

Component* EmbedCircuit::getMainComp( QString uid )
{
    if( m_mainComponents.contains( uid ) )        // Found in list
        return m_mainComponents.value( uid );

    QString type = uid.split("-").last();
    for( QString cUid : m_mainComponents.keys() ) // Not found by Uid, search by type
        if( cUid.split("-").last() == type )
            return m_mainComponents.value( cUid );

    if( m_mainComponents.size() )
        return m_mainComponents.values().first(); // Not found by type, return the first one

    return nullptr;                               // Not found at all
}

QString EmbedCircuit::toString()
{
    QString item;
    QString end = " />\n";

    if( !m_mainComponents.isEmpty() )
    {
        item.remove( end );
        item += ">";

        for( QString uid : m_mainComponents.keys() )
        {
            Component* mainComponent = m_mainComponents.value( uid );
            item += "\n<mainCompProps MainCompId=\""+uid+"\" ";
            for( propGroup pg : *mainComponent->properties() )
            {
                if( pg.flags & groupNoCopy ) continue;

                for( ComProperty* prop : pg.propList )
                {
                    QString val = prop->toString();
                    if( val.isEmpty() ) continue;
                    item += prop->name() + "=\""+val+"\" ";
            }   }
            item += "/>\n";
        }
        item += "</item>\n";
    }
    return item;
}

void EmbedCircuit::loadGraphProps()
{
    propGroup* pg = m_component->getPropGroup( "CompGraphic" ); // Create list of "Graphical" poperties (We don't need them)
    for( ComProperty* prop : pg->propList ) s_graphProps.append( prop->name() );
}
