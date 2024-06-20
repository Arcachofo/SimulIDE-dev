/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>

#include "subcircuit.h"
#include "itemlibrary.h"
#include "mainwindow.h"
#include "componentlist.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "tunnel.h"
#include "node.h"
#include "utils.h"
#include "mcu.h"
#include "linker.h"

#include "logicsubc.h"
#include "board.h"
#include "shield.h"
#include "module.h"

#include "stringprop.h"
//#include "boolprop.h"

#define tr(str) simulideTr("SubCircuit",str)

QString SubCircuit::m_subcDir = "";

Component* SubCircuit::construct( QString type, QString id )
{
    m_error = 0;
    m_subcDir = "";

    QString name;
    QStringList list = id.split("-");
    if( list.size() > 1 ){
        name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"
        list.takeLast();
    }

    int rev = Circuit::self()->circuitRev();
    if( rev >= 2220 ){ if( name.contains("@") ) list = name.split("@");}
    else if( name.contains("_") ) list = name.split("_");

    QMap<QString, QString> packageList;
    QString subcTyp = "None";
    QString subcDoc;
    QString pkgeFile;
    QString subcFile;

    if( list.size() > 1 )  // Subcircuit inside Subcircuit: 1_74HC00 to 74HC00
    {
        QString n = list.first();
        bool ok = false;
        n.toInt(&ok);
        if( ok ) name = list.at( 1 );
    }
    QString dataFile = ComponentList::self()->getDataFile( name );

    if( dataFile.isEmpty() ) // Component not installed, search in Circuit folder
    {
        dataFile = MainWindow::self()->getDataFilePath( name+".comp" );
        if( !QFile::exists( dataFile ) ) dataFile = "";
    }

    if( dataFile.isEmpty() ) // use old system
    {
        m_subcDir = ComponentList::self()->getFileDir( name ); // Found in folder (no xml file)
        if( m_subcDir.isEmpty() )                                  // Try to find a "data" folder in Circuit folder
        {
            m_subcDir = MainWindow::self()->getDataFilePath( name );
            //QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
            //m_subcDir = circuitDir.absoluteFilePath( "data/"+name );
        }
    }
    else if( dataFile.endsWith(".comp") ) // Subcircuit in single file (.comp)
    {
        subcFile = dataFile;
        QStringList list = fileToStringList( dataFile, "SubCircuit::construct" );
        //list.takeFirst(); // Remove first line: <circuit or <libitem

        for( QString line : list )
        {
            if( !line.startsWith("<item") ) continue;
            //line.replace("&#x3D;","=");
            //line.replace("&lt","&#x3E;");

            QDomDocument domDoc;
            domDoc.setContent( line );
            QDomElement root = domDoc.documentElement();

            QString itemType = root.attribute("itemtype");
            if( itemType != "Package") break;

            QString type = root.attribute("SubcType");
            if( type != "None" ) subcTyp = type;

            QString pkgName = root.attribute("label");
            packageList[pkgName] = convertPackage( line );
        }
    }
    else                                  // Subcircuit listed in xml file
    {
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
                    m_subcDir = MainWindow::self()->getDataFilePath( folder+"/"+name );
                    found = true;
                }
                if( found ) break;
                node = node.nextSibling();
            }
            if( found ) break;
            rNode = rNode.nextSibling();
        }
    }

    if( !m_subcDir.isEmpty() ) // Packages from package files
    {
        subcFile  = m_subcDir+"/"+name+".sim1";
        pkgeFile  = m_subcDir+"/"+name+".package";
        QString pkgFileLS = m_subcDir+"/"+name+"_LS.package";
        QString pkgName   = "2- DIP";
        QString pkgNameLS = "1- Logic Symbol";

        bool dip = QFile::exists( pkgeFile );
        bool ls  = QFile::exists( pkgFileLS );
        if( !dip && !ls ){
            qDebug() << "SubCircuit::construct: Error No package files found for "<<name<<endl;
            return NULL;
        }

        if( dip ){
            QString pkgStr = fileToString( pkgeFile, "SubCircuit::construct" );
            packageList[pkgName] = convertPackage( pkgStr );
        }
        else pkgeFile = pkgFileLS; // If no DIP package file then use LS

        if( ls ){
            QString pkgStr = fileToString( pkgFileLS, "SubCircuit::construct" );
            packageList[pkgNameLS] = convertPackage( pkgStr );
        }

        QDomDocument domDoc1 = fileToDomDoc( pkgeFile, "SubCircuit::construct" );
        QDomElement   root1  = domDoc1.documentElement();
        if( root1.hasAttribute("type") ) subcTyp = root1.attribute("type").remove("subc");

        if( packageList.size() > 1 && subcTyp != "None" && subcTyp != "Logic" ) // For Boards insert LS last
        {
            QMap<QString, QString> pList;

            QString pkg = packageList.value( pkgName );
            if( !pkg.isEmpty() ) pList["1- DIP"] = pkg;

            pkg = packageList.value( pkgNameLS );
            if( !pkg.isEmpty() ) pList["2- Logic Symbol"] = pkg; // LS last

            packageList = pList;
        }
    }

    if( packageList.isEmpty() ){
        qDebug() << "SubCircuit::construct: No Packages found for"<<name<<endl;
        return NULL;
    }

    SubCircuit* subcircuit = NULL;
    if     ( subcTyp == "Logic"  ) subcircuit = new LogicSubc( type, id );
    else if( subcTyp == "Board"  ) subcircuit = new BoardSubc( type, id );
    else if( subcTyp == "Shield" ) subcircuit = new ShieldSubc( type, id );
    else if( subcTyp == "Module" ) subcircuit = new ModuleSubc( type, id );
    else                           subcircuit = new SubCircuit( type, id );

    if( m_error != 0 )
    {
        subcircuit->remove();
        m_error = 0;
        return NULL;
    }else{
        Circuit::self()->m_createSubc = true;

        QStringList pkges = packageList.keys();
        subcircuit->m_packageList = packageList;
        subcircuit->m_enumUids = pkges;
        subcircuit->m_enumNames = subcircuit->m_enumUids;
        subcircuit->m_dataFile = subcFile;

        if( packageList.size() > 1 ) // Add package list if there is more than 1 to choose
        subcircuit->addProperty( tr("Main"),
        new StrProp <SubCircuit>("Package", tr("Package"),""
                                , subcircuit, &SubCircuit::package, &SubCircuit::setPackage,0,"enum" ));

        subcircuit->setPackage( pkges.first() );
        if( m_error == 0 ) subcircuit->loadSubCircuitFile( subcFile );

        Circuit::self()->m_createSubc = false;
    }
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subcircuit );
        delete subcircuit;
        m_error = 0;
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

SubCircuit::SubCircuit( QString type, QString id )
          : Chip( type, id )
{
    m_lsColor = QColor( 235, 240, 255 );
    m_icColor = QColor( 20, 30, 60 );

    addPropGroup( { tr("Main"), {},0} );
}
SubCircuit::~SubCircuit(){}

void SubCircuit::loadSubCircuitFile( QString file )
{
    QString doc = fileToString( file, "SubCircuit::loadSubCircuit" );

    /// FIXME: Subcircuit loaded from .comp
    QString oldFilePath = Circuit::self()->getFilePath();
    Circuit::self()->setFilePath( file );             // Path to find subcircuits/Scripted in our data folder

    loadSubCircuit( doc );

    Circuit::self()->setFilePath( oldFilePath ); // Restore original filePath
}

void SubCircuit::loadSubCircuit( QString doc )
{
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

    QList<Linker*> linkList;   // Linked  Component list

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
                    if     ( name == "itemtype"  ) type  = prop.toString();
                    else if( name == "CircId"    ) uid   = prop.toString();
                    else if( name == "objectName") uid   = prop.toString();
                    else if( name == "label"     ) label = prop.toString();
                    else if( name == "id"        ) label = prop.toString();
                    else properties << name << prop ;
            }   }
            newUid = numId+"@"+uid;

            if( type == "Package" || type == "Subcircuit" ) continue;

            if( type == "Connector" )
            {
                QString startPinId, endPinId, enodeId;
                QStringList pointList;

                QString name = "";
                for( QStringRef prop : properties )
                {
                    if( name.isEmpty() ) { name = prop.toString(); continue; }

                    if     ( name == "startpinid") startPinId = numId+"@"+prop.toString();
                    else if( name == "endpinid"  ) endPinId   = numId+"@"+prop.toString();
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
            else{
                Component* comp = NULL;

                if( type == "Node" ) comp = new Node( type, newUid );
                else                 comp = circ->createItem( type, newUid, false );

                if( comp ){
                    comp->setIdLabel( uid ); // Avoid parent Uids in label

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
                        /*if( pos == QPointF(-1e6,-1e6 ) ) // Don't show Components not placed
                        {
                            pos = QPointF( 0, 0 );
                            comp->setVisible( false );
                        }*/
                        comp->moveTo( pos );
                        comp->setRotation( comp->boardRot() );
                        comp->setHflip( comp->boardHflip() );
                        comp->setVflip( comp->boardVflip() );
                        if( !this->collidesWithItem( comp ) ) // Don't show Components out of Board
                        {
                            comp->setBoardPos( QPointF(-1e6,-1e6 ) ); // Used in setLogicSymbol to identify Components not visible
                            comp->moveTo( QPointF( 0, 0 ) );
                            comp->setVisible( false );
                        }
                        comp->setHidden( true, true, true ); // Boards: hide non graphical
                        if( m_isLS && m_packageList.size() > 1 ) comp->setVisible( false ); // Don't show any component if Logic Symbol
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
                        tunnel->setName( m_id+"-"+tunnel->name() );
                        m_subcTunnels.append( tunnel );
                }   }
                else qDebug() << "SubCircuit:"<<m_name<<m_id<< "ERROR Creating Component: "<<type<<uid<<label;
    }   }   }
    for( Linker* l : linkList ) l->createLinks( &m_compList );
}

Pin* SubCircuit::addPin( QString id, QString type, QString label, int, int xpos, int ypos, int angle, int length, int space )
{
    if( m_initialized && m_pinTunnels.contains( m_id+"-"+id ) )
    {
        return updatePin( id, type, label, xpos, ypos, angle, length );
    }else{
        QColor color = Qt::black;
        if( !m_isLS ) color = QColor( 250, 250, 200 );

        QString pId = m_id+"-"+id;
        Tunnel* tunnel = new Tunnel("Tunnel", pId );
        m_compList.append( tunnel );

        tunnel->setParentItem( this );
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
        addSignalPin( pin );

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
}

Pin* SubCircuit::updatePin( QString id, QString type, QString label, int xpos, int ypos, int angle, int length, int space )
{
    Pin* pin = NULL;
    Tunnel* tunnel = m_pinTunnels.value( m_id+"-"+id );
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
    if( unused && m_isLS )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
        return pin;
    }
    if( m_isLS ) pin->setLabelColor( QColor( 0, 0, 0 ) );
    else         pin->setLabelColor( QColor( 250, 250, 200 ) );

    pin->setInverted( type == "inverted" || type == "inv" );
    pin->setLength( length );
    pin->setSpace( space );
    pin->setLabelText( label );
    pin->setVisible( true );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );
    pin->isMoved();

    return pin;
}

void SubCircuit::setLogicSymbol( bool ls )
{
    Chip::setLogicSymbol( ls );

    if( m_isLS )
    {
        for( QString tNam : m_pinTunnels.keys() )   // Don't show unused Pins in LS
        {
            Tunnel* tunnel = m_pinTunnels.value( tNam );
            Pin* pin = tunnel->getPin();
            if( pin->unused() ) { pin->setVisible( false ); pin->setLabelText( "" ); }
        }
        if( m_backPixmap )    // No background image in LS
        {
            delete m_backPixmap;
            m_backPixmap = nullptr;
        }
    }
    for( Component* comp : m_compList ) // Don't show graphical components in LS if Board
    {
        if( !comp->isGraphical() ) continue;
        if( m_subcType >= Board )
        {
            comp->setVisible( !m_isLS && comp->boardPos() != QPointF(-1e6,-1e6 ) );
        }
        else if( m_isLS )  comp->setVisible( false );
    }
}

Component* SubCircuit::getMainComp( QString uid )
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

void SubCircuit::remove()
{
    for( Component* comp : m_compList ) comp->remove();
    m_pin.clear();
    Component::remove();
}

void SubCircuit::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    event->accept();

    addMainCompsMenu( menu );

    Component::contextMenu( event, menu );
}

void SubCircuit::addMainCompsMenu( QMenu* menu )
{
    for( Component* mainComp : m_mainComponents.values() )
    {
        QString name = mainComp->idLabel();
        QMenu* submenu = menu->addMenu( QIcon(":/subc.png"), name );
        mainComp->contextMenu( NULL, submenu );
    }
    menu->addSeparator();
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
