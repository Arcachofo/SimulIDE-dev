/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
#include "mcu.h"
#include "linkable.h"

#include "logicsubc.h"
#include "board.h"
#include "shield.h"
#include "module.h"

#include "boolprop.h"

QString SubCircuit::m_subcDir = "";

Component* SubCircuit::construct( QObject* parent, QString type, QString id )
{
    m_error = 0;
    m_subcDir = "";

    QString name;
    QStringList list = id.split("-");
    if( list.size() > 1 ) name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"

    list = name.split("_");
    if( list.size() > 1 )  // Subcircuit inside Subcircuit: 1_74HC00 to 74HC00
    {
        QString n = list.first();
        bool ok = false;
        n.toInt(&ok);
        if( ok ) name = list.at( 1 );
    }
    QString dataFile = ComponentSelector::self()->getXmlFile( name );

    if( dataFile == "" ) // Component is not in SimulIDE, search in Circuit folder
    {
        m_subcDir = ComponentSelector::self()->getFileDir( name ); // Found in folder (no xml file)
        if( m_subcDir.isEmpty() )                                  // Try to find a "data" folder in Circuit folder
        {
            QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
            m_subcDir = circuitDir.absoluteFilePath( "data/"+name );
        }
    }
    else{
        QDomDocument domDoc = fileToDomDoc( dataFile, "SubCircuit::construct");
        if( domDoc.isNull() ) return NULL; // m_error = 1;

        QDomElement root  = domDoc.documentElement();
        QDomNode    rNode = root.firstChild();

        bool found = false;
        while( !rNode.isNull() )
        {
            QDomElement itemSet = rNode.toElement();
            QDomNode    node    = itemSet.firstChild();

            QString folder = "";
            if( itemSet.hasAttribute("folder") ) folder = itemSet.attribute("folder");

            while( !node.isNull() )
            {
                QDomElement element = node.toElement();

                if( element.attribute("name") == name )
                {
                    if( element.hasAttribute("folder") ) folder = element.attribute("folder");
                    QFileInfo fi( dataFile );
                    m_subcDir = fi.absolutePath()+"/"+folder+"/"+name;
                    found = true;
                }
                if( found ) break;
                node = node.nextSibling();
            }
            if( found ) break;
            rNode = rNode.nextSibling();
        }
    }
    if( m_subcDir.isEmpty() ){
        qDebug() << "SubCircuit::construct: No Circuit files found for"+name+"\n";
        return NULL;
    }
    QString pkgeFile  = m_subcDir+"/"+name+".package";
    QString pkgFileLS = m_subcDir+"/"+name+"_LS.package";
    QString subcFile  = m_subcDir+"/"+name+".sim1";

    bool dip = QFile::exists( pkgeFile );
    bool ls  = QFile::exists( pkgFileLS );

    if( !dip ){        // Check if package file exist, if not try LS
        if( !ls ){
            qDebug() << "SubCircuit::construct: No package files found for "+name+"\n"+pkgeFile+"\n";
            return NULL;
        }
        pkgeFile = pkgFileLS;
    }
    QDomDocument domDoc1 = fileToDomDoc( pkgeFile, "SubCircuit::construct" );
    QDomElement   root1  = domDoc1.documentElement();

    QString subcTyp = "None";
    if( root1.hasAttribute("type") ) subcTyp = root1.attribute("type").remove("subc");

    SubCircuit* subcircuit = NULL;
    if     ( subcTyp == "Logic"  ) subcircuit = new LogicSubc( parent, type, id );
    else if( subcTyp == "Board"  ) subcircuit = new BoardSubc( parent, type, id );
    else if( subcTyp == "Shield" ) subcircuit = new ShieldSubc( parent, type, id );
    else if( subcTyp == "Module" ) subcircuit = new ModuleSubc( parent, type, id );
    else                           subcircuit = new SubCircuit( parent, type, id );

    if( m_error != 0 )
    {
        subcircuit->remove();
        return NULL;
    }else{
        Circuit::self()->m_createSubc = true;
        subcircuit->m_pkgeFile = pkgeFile;
        subcircuit->initChip();
        if( m_error == 0 ) subcircuit->loadSubCircuit( subcFile );

        if( dip && ls ) // If no both files exist, this prop. is not needed
        subcircuit->addPropGroup( { tr("Main"), {
        new BoolProp<SubCircuit>( "Logic_Symbol", tr("Logic Symbol"),"", subcircuit, &SubCircuit::logicSymbol, &SubCircuit::setLogicSymbol ),
        },groupNoCopy} );
        Circuit::self()->m_createSubc = false;
    }
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subcircuit );
        subcircuit->deleteLater();
        return NULL;
    }
    return subcircuit;
}

LibraryItem* SubCircuit::libraryItem()
{
    return new LibraryItem(
        tr("Subcircuit"),
        "",         // Category Not dispalyed
        "",
        "Subcircuit",
        SubCircuit::construct );
}

SubCircuit::SubCircuit( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    m_icColor = QColor( 20, 30, 60 );
    //m_mainComponent = NULL;
}
SubCircuit::~SubCircuit(){}

void SubCircuit::loadSubCircuit( QString fileName )
{
    QString doc = fileToString( fileName, "SubCircuit::loadSubCircuit" );

    QString oldFilePath = Circuit::self()->getFilePath();
    Circuit::self()->setFilePath( fileName );             // Path to find subcircuits/Scripted in our data folder

    QStringList graphProps;
    for( propGroup pg : m_propGroups ) // Create list of "Graphical" poperties (We don't need them)
    {
        if( (pg.name != "CompGraphic") ) continue;
        for( ComProperty* prop : pg.propList ) graphProps.append( prop->name() );
        break;
    }

    QString numId = m_id;
    numId = numId.split("-").last();
    Circuit* circ = Circuit::self();

    QList<Linkable*> linkList;   // Linked  Component list

    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.startsWith("<item") )
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
            newUid = numId+"_"+uid;

            if( type == "Connector" )
            {
                QString startPinId, endPinId, enodeId;
                QStringList pointList;

                QString name = "";
                for( QStringRef prop : properties )
                {
                    if( name.isEmpty() ) { name = prop.toString(); continue; }

                    if     ( name == "startpinid") startPinId = numId+"_"+prop.toString();
                    else if( name == "endpinid"  ) endPinId   = numId+"_"+prop.toString();
                    else if( name == "enodeid"   ) enodeId    = prop.toString();
                    else if( name == "pointList" ) pointList  = prop.toString().split(",");
                    name = "";
                }
                startPinId = startPinId.replace("Pin-", "Pin_"); // Old TODELETE
                endPinId   =   endPinId.replace("Pin-", "Pin_"); // Old TODELETE

                Pin* startPin = circ->m_LdPinMap.value( startPinId );
                Pin* endPin   = circ->m_LdPinMap.value( endPinId );

                if( startPin && endPin )    // Create Connector
                {
                    startPin->setConPin( endPin );
                    endPin->setConPin( startPin );
                }
                else // Start or End pin not found
                {
                    if( !startPin ) qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_name<<m_id+" null startPin in "<<type<<uid<<startPinId;
                    if( !endPin )   qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_name<<m_id+" null endPin in "  <<type<<uid<<endPinId;
            }   }
            else if( type == "Package" ) { ; }
            else{
                Component* comp = NULL;

                if( type == "Node" ) comp = new Node( this, type, newUid );
                else                 comp = circ->createItem( type, newUid );

                if( comp ){
                    QString propName = "";
                    for( QStringRef prop : properties )
                    {
                        if( propName.isEmpty() ) { propName = prop.toString(); continue; }
                        if( !graphProps.contains( propName ) ) comp->setPropStr( propName, prop.toString() );
                        propName = "";
                    }
                    comp->setParentItem( this );

                    if( m_subcType >= Board && comp->isGraphical() )
                    {
                        QPointF pos = comp->boardPos();
                        if( pos == QPointF( -1e6, -1e6 ) ) // Don't show Components not placed
                        {
                            pos = QPointF( 0, 0 );
                            comp->setVisible( false );
                        }
                        comp->moveTo( pos );
                        comp->setRotation( comp->boardRot() );
                        if( !this->collidesWithItem( comp ) ) // Don't show Components out of Board
                        {
                            pos = QPointF( 0, 0 );
                            comp->moveTo( pos );
                            comp->setVisible( false );
                        }
                        comp->setHidden( true, true, true ); // Boards: hide non graphical
                    }
                    else{
                        comp->moveTo( QPointF(20, 20) );
                        comp->setVisible( false );     // Not Boards: Don't show any component
                    }

                    if( comp->itemType() == "MCU" )
                    {
                        comp->removeProperty("Logic_Symbol");
                        Mcu* mcu = (Mcu*)comp;
                        QString program = mcu->program();
                        if( !program.isEmpty() ) mcu->load( m_subcDir+"/"+program );
                    }
                    if( comp->isMainComp() )
                    {
                        m_mainComponents[newUid] = comp; // This component will add it's Context Menu and properties

                        QList<propGroup>* props = comp->properties();
                        for( propGroup pg : *props )
                        {
                            if( pg.flags & groupNoCopy ) continue;

                            propGroup npg;
                            int len = label.lastIndexOf("-");
                            npg.name = label.mid( 0, len )+" "+pg.name;
                            npg.flags = pg.flags;

                            for( ComProperty* p : pg.propList ) // Copy properties
                            {
                                if( p->flags() & propNoCopy ) continue;
                                npg.propList.append( p );
                            }
                            addPropGroup( npg, false );
                        }
                    }
                    m_compList.append( comp );

                    if( comp->m_linkable )
                    {
                        Linkable* l = dynamic_cast<Linkable*>(comp);
                        if( l->hasLinks() ) linkList.append( l );
                    }

                    if( type == "Tunnel" ) // Make Tunnel names unique for this subcircuit
                    {
                        Tunnel* tunnel = static_cast<Tunnel*>( comp );
                        tunnel->setTunnelUid( tunnel->name() );
                        tunnel->setName( m_id+"-"+tunnel->name() );
                        m_subcTunnels.append( tunnel );
                }   }
                else qDebug() << "SubCircuit:"<<m_name<<m_id<< "ERROR Creating Component: "<<type<<uid<<label;
    }   }   }
    for( Linkable* l : linkList )
        l->createLinks( &m_compList );

    Circuit::self()->setFilePath( oldFilePath ); // Restore original filePath
}

Pin* SubCircuit::addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length )
{
    if( m_initialized && m_pinTunnels.contains( m_id+"-"+id ) )
    {
        return updatePin( id, type, label, xpos, ypos, angle, length );
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
        tunnel->setTunnelUid( id );
        tunnel->setName( pId ); // Make tunel name unique for this component
        tunnel->setPos( xpos, ypos );
        tunnel->setPacked( true );
        m_pinTunnels.insert( pId, tunnel );

        Pin* pin = tunnel->getPin();
        /// pin->setObjectName( pId );
        pin->setId( pId );
        pin->setInverted( type == "inverted" || type == "inv" );
        addSignalPin( pin );
        /// connect( this, &SubCircuit::moved, pin, &Pin::isMoved, Qt::UniqueConnection );

        tunnel->setRotated( angle >= 180 );      // Our Pins at left side
        if     ( angle == 180) tunnel->setRotation( 0 );
        else if( angle == 90 ) tunnel->setRotation( -90 ); // QGraphicsItem 0º i at right side
        else                   tunnel->setRotation( angle );

        pin->setLength( length );
        pin->setLabelColor( color );
        pin->setLabelText( label );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        return pin;
    }
}

Pin* SubCircuit::updatePin( QString id, QString type, QString label, int xpos, int ypos, int angle, int length )
{
    Pin* pin = NULL;
    Tunnel* tunnel = m_pinTunnels.value( m_id+"-"+id );
    if( !tunnel )
    {
        qDebug() <<"SubCircuit::updatePin Pin Not Found:"<<id<<type<<label;
        return NULL;
    }
    tunnel->setPos( xpos, ypos );
    tunnel->setRotated( angle >= 180 );      // Our Pins at left side

    if     ( angle == 180) tunnel->setRotation( 0 );
    else if( angle == 90 ) tunnel->setRotation( -90 ); // QGraphicsItem 0º i at right side
    else                   tunnel->setRotation( angle );

    pin  = tunnel->getPin();
    type = type.toLower();

    if( m_isLS ) pin->setLabelColor( QColor( 0, 0, 0 ) );
    else         pin->setLabelColor( QColor( 250, 250, 200 ) );

    if( type == "unused" || type == "nc" )
    {
        pin->setUnused( true );
        if( m_isLS )
        {
            pin->setVisible( false );
            pin->setLabelText( "" );
        }
    }
    pin->setInverted( type == "inverted" || type == "inv" );
    pin->setLength( length );
    pin->setLabelText( label );
    pin->setVisible( true );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );
    pin->isMoved();

    return pin;
}

void SubCircuit::setLogicSymbol( bool ls )
{
    if( !m_initialized ) return;
    if( m_isLS == ls ) return;

    Chip::setLogicSymbol( ls );
    if( m_isLS != ls ) return; // Change not done

    if( m_isLS )               // Don't show unused Pins in LS
    {
        for( QString tNam : m_pinTunnels.keys() )
        {
            Tunnel* tunnel = m_pinTunnels.value( tNam );
            Pin* pin = tunnel->getPin();
            if( pin->unused() ) { pin->setVisible( false ); pin->setLabelText( "" ); }
        }
    }
    if( m_subcType >= Board ) // Don't show graphical components in LS if Board
    {
        for( Component* c : m_compList )
            if( c->isGraphical() ) c->setVisible( !m_isLS );
    }
}

Component* SubCircuit::getMainComp( QString name )
{
    if( name.isEmpty() ) return m_mainComponents.value( m_mainComponents.keys().first() );
    return m_mainComponents.value(name);
}

void SubCircuit::remove()
{
    for( Component* comp : m_compList )
    {
        comp->setParentItem( NULL );
        Circuit::self()->removeComp( comp );
    }
    m_pin.clear();
    Component::remove();
}

void SubCircuit::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    event->accept();

    for( Component* mainComp : m_mainComponents.values() )
    {
        QString compType = mainComp->getUid();
        int pos = compType.indexOf("_")+1;
        int len = compType.lastIndexOf("-")-pos;
        compType = compType.mid( pos, len );

        QMenu* submenu = menu->addMenu( QIcon(":/subc.png"), compType );

        mainComp->contextMenu( NULL, submenu );
    }
    menu->addSeparator();
    Component::contextMenu( event, menu );
}

QString SubCircuit::toString()
{
    QString item = CompBase::toString();
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
