/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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

#define tr(str) simulideTr("SubCircuit",str)

QString SubCircuit::m_subcDir = "";
QStringList SubCircuit::s_graphProps;

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
    QString dataFile;

    if( list.size() > 1 )  // Subcircuit inside Subcircuit: 1_74HC00 to 74HC00
    {
        QString n = list.first();
        bool ok = false;
        n.toInt(&ok);
        if( ok ) name = list.at( 1 );
    }

    m_subcDir = MainWindow::self()->getCircFilePath( name ); // Search subc folder in circuit/data folder

    if( m_subcDir.isEmpty() )
        m_subcDir = ComponentList::self()->getFileDir( name ); // Get subc folder from list
    else
        dataFile = MainWindow::self()->getCircFilePath( name+"/"+name+".sim1" ); // Search sim1 in circuit/data/name folder

    if( dataFile.isEmpty() )
        dataFile = MainWindow::self()->getCircFilePath( name+".sim1" ); // Search sim1 in circuit/data folder

    if( dataFile.isEmpty() ) // Get sim1 from list
        dataFile = ComponentList::self()->getDataFile( name );

    if( dataFile.endsWith(".sim1")) subcFile = dataFile;
    else if( !m_subcDir.isEmpty() ) subcFile = m_subcDir+"/"+name+".sim1";

    if( !subcFile.isEmpty() ){
        packageList = getPackages( subcFile ); // Try packages from sim1 file
        subcTyp = s_subcType;

        if( packageList.isEmpty() ) // Packages from package files
        {
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

            Chip::s_subcType = "None";
            if( dip ){
                QString pkgStr = fileToString( pkgeFile, "SubCircuit::construct" );
                packageList[pkgName] = convertPackage( pkgStr );
                subcTyp = s_subcType;
            }
            if( ls ){
                QString pkgStr = fileToString( pkgFileLS, "SubCircuit::construct" );
                packageList[pkgNameLS] = convertPackage( pkgStr );
                if( subcTyp == "None" ) subcTyp = s_subcType;
            }
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

    if( s_graphProps.isEmpty() ) loadGraphProps();
}
SubCircuit::~SubCircuit(){}

void SubCircuit::loadSubCircuitFile( QString file )
{
    QString doc = fileToString( file, "SubCircuit::loadSubCircuit" );

    QString oldFilePath = Circuit::self()->getFilePath();
    Circuit::self()->setFilePath( file );             // Path to find subcircuits/Scripted in our data folder

    loadSubCircuit( doc );

    Circuit::self()->setFilePath( oldFilePath ); // Restore original filePath
}

void SubCircuit::loadSubCircuit( QString doc )
{
    QString numId = m_id;
    numId = numId.split("-").last();
    Circuit* circ = Circuit::self();

    QList<Linker*> linkList;   // Linked  Component list

    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.startsWith("<item") )
        {
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
                //startPinId = startPinId.replace("Pin-", "Pin_"); // Old TODELETE
                //endPinId   =   endPinId.replace("Pin-", "Pin_"); // Old TODELETE

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
                    if( !startPin ) qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_name<<m_id+" null startPin in "<<type<<startPinId;
                    if( !endPin )   qDebug()<<"\n   ERROR!!  SubCircuit::loadSubCircuit: "<<m_name<<m_id+" null endPin in "  <<type<<endPinId;
            }   }
            else{
                Component* comp = NULL;

                propStr_t circId = properties.takeFirst();
                if( circId.name != "CircId") continue; /// ERROR
                QString uid = circId.value.toString();
                QString newUid = numId+"@"+uid;

                if( type == "Node" ) comp = new Node( type, newUid );
                else                 comp = circ->createItem( type, newUid, false );

                if( comp ){
                    comp->setIdLabel( uid ); // Avoid parent Uids in label

                    for( propStr_t prop : properties )
                    {
                        QString propName = prop.name.toString();
                        if( !s_graphProps.contains( propName ) ) comp->setPropStr( propName, prop.value.toString() );
                    }
                    comp->setup();
                    comp->setParentItem( this );

                    if( m_subcType >= Board && comp->isGraphical() )
                    {
                        QPointF pos = comp->boardPos();

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
                        comp->remProperty("Logic_Symbol");
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
                else qDebug() << "SubCircuit:"<<m_name<<m_id<< "ERROR Creating Component: "<<type<<uid;
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

Pin* SubCircuit::findPin( QString pinId )
{
    QStringList words = pinId.split("-");
    pinId = words.takeLast();
    QString compId = words.join("-");

    for( Component* comp : m_compList ) if( comp->getUid() == compId ) return comp->getPin( pinId );

    return nullptr;
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

void SubCircuit::loadGraphProps()
{
    for( propGroup pg : m_propGroups ) // Create list of "Graphical" poperties (We don't need them)
    {
        if( (pg.name != "CompGraphic") ) continue;
        for( ComProperty* prop : pg.propList ) s_graphProps.append( prop->name() );
        break;
    }
}
