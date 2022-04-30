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

#include "logicsubc.h"
#include "board.h"
#include "shield.h"
#include "module.h"

#include "boolprop.h"

Component* SubCircuit::construct( QObject* parent, QString type, QString id )
{
    m_error = 0;

    QString name;
    QStringList list = id.split("-");
    if( list.size() > 1 ) name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"

    QString pkgeFile;
    QString subcFile;
    QString dataFile = ComponentSelector::self()->getXmlFile( name );

    if( dataFile == "" ){
          MessageBoxNB( "SubCircuit::construct", "                               \n"+
                    tr( "There are no data files for " )+name+"    ");
          //m_error = 23;
          return NULL;
    }
    QDomDocument domDoc = fileToDomDoc( dataFile, "SubCircuit::construct" );
    if( domDoc.isNull() ) return NULL; // m_error = 1;

    QDomElement root  = domDoc.documentElement();
    QDomNode    rNode = root.firstChild();

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

            if( element.attribute("name") == name )
            {
                QDir dataDir( dataFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name

                QString path = "";

                if( folder != "" )
                {
                    path = folder+"/"+name+"/"+name;
                    pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".sim1" );
                }
                if( element.hasAttribute( "folder") )
                {
                    path = element.attribute( "folder" )+"/"+name+"/"+name;
                    pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".sim1" );
                }

                if( element.hasAttribute( "package") )
                    pkgeFile = dataDir.filePath( element.attribute( "package" ) );

                if( !pkgeFile.endsWith( ".package" ) ) pkgeFile += ".package" ;

                if( element.hasAttribute( "subcircuit") )
                {
                    subcFile = dataDir.filePath( element.attribute( "subcircuit" ) );
                    if( subcFile.endsWith( ".simu" ) ) subcFile= changeExt( subcFile, ".sim1" );
                    if( !subcFile.endsWith( ".sim1" )) subcFile += ".sim1" ;
                }
                if( !QFile::exists( subcFile) ) subcFile = changeExt( subcFile, ".simu" );

                /*if( m_mainComponent )  // Example MCU in subcircuit needs to know where subcircuit is.
                {
                    dataDir.setPath( subcFile );
                    dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                    m_mainComponent->setSubcDir( dataDir.absolutePath() );
                    m_mainComponent->m_subcircuit = this;
                }*/
                break;
            }
            node = node.nextSibling();
        }
        rNode = rNode.nextSibling();
    }


    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( pkgeFile );

    QFile pfile( fileNameAbs );
    if( !pfile.exists() )   // Check if package file exist, if not try LS or no LS
    {
        if     ( pkgeFile.endsWith("_LS.package")) pkgeFile.replace( "_LS.package", ".package" );
        else if( pkgeFile.endsWith(".package"))    pkgeFile.replace( ".package", "_LS.package" );
        else{
            //m_error = 1;
            qDebug() << "SubCircuit::construct: No package files found.\n";
            return NULL;
        }
        fileNameAbs = circuitDir.absoluteFilePath( pkgeFile );
    }

    QDomDocument domDoc1 = fileToDomDoc( fileNameAbs, "SubCircuit::construct" );
    QDomElement   root1  = domDoc1.documentElement();

    QString subcTyp = "None";
    if( root1.hasAttribute("type") ) subcTyp = root1.attribute("type").remove("subc");



    SubCircuit* subcircuit = NULL;
    if     ( subcTyp == "None"  )  subcircuit = new SubCircuit( parent, type, id );
    else if( subcTyp == "Logic" )  subcircuit = new LogicSubc( parent, type, id );
    else if( subcTyp == "Board" )  subcircuit = new BoardSubc( parent, type, id );
    else if( subcTyp == "Shield" ) subcircuit = new ShieldSubc( parent, type, id );
    else if( subcTyp == "Module" ) subcircuit = new ModuleSubc( parent, type, id );
    else return NULL; //m_error = 1;

    if( m_error != 0 )
    {
        subcircuit->remove();
        return NULL;
    }else{
        subcircuit->m_pkgeFile = pkgeFile;
        subcircuit->loadSubCircuit( subcFile );
        subcircuit->initChip();
    }

    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subcircuit );
        subcircuit->deleteLater();
        subcircuit = NULL;
        m_error = 0;
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
    m_mainComponent = NULL;

    addPropGroup( { tr("Main"), {
    new BoolProp<SubCircuit>( "Logic_Symbol", tr("Logic Symbol"),"", this, &SubCircuit::logicSymbol, &SubCircuit::setLogicSymbol ),
    }} );
}
SubCircuit::~SubCircuit(){}

void SubCircuit::loadSubCircuit( QString fileName )
{
    QString doc = fileToString( fileName, "SubCircuit::loadSubCircuit" );

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

                Pin* startPin = Circuit::self()->m_LdPinMap.value( startPinId ); //getConPin( startPinId );
                Pin* endPin   = Circuit::self()->m_LdPinMap.value( endPinId ); //getConPin( endPinId );

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
                        QString value = prop.toString();

                        if( !graphProps.contains( propName ) ){
                            if( !comp->setPropStr( propName, value ) ) // SUBSTITUTIONS
                            {
                                if( propName == "Propagation_Delay_ns") { propName = "Tpd_ps"; value.append("000"); } // ns to ps
                                else                                    Component::substitution( propName );

                                if( !comp->setPropStr( propName, value ) ){
                                    if( propName.toLower()  != "tristate" )   // TODELETE
                                        qDebug() << "SubCircuit:"<<m_name<<m_id<<"Wrong Property: "<<type<<uid<<propName<<value; }
                            }
                        }
                        propName = "";
                    }
                    comp->setParentItem( this );

                    if( comp->isGraphical() )
                    {
                        QPointF pos = comp->boardPos();
                        if( pos == QPointF( -1e6, -1e6 ) ) pos = QPointF( 0, 0 );
                        comp->moveTo( pos );
                        comp->setRotation( comp->boardRot() );
                    }
                    else comp->moveTo( QPointF(20, 20) );

                    if( comp->isMainComp() )
                    {
                        m_mainComponent = comp; // This component will add it's Context Menu
                        qDebug() <<comp->itemType();
                        if( comp->itemType() == "MCU" ) comp->removeProperty( "Main", "Logic_Symbol" );
                    }
                    comp->setHidden( true, true );
                    m_compList.append( comp );

                    if( type == "Tunnel" ) // Make Tunnel names unique for this subcircuit
                    {
                        Tunnel* tunnel = static_cast<Tunnel*>( comp );
                        tunnel->setTunnelUid( tunnel->name() );
                        tunnel->setName( m_id+"-"+tunnel->name() );
                        m_subcTunnels.append( tunnel );
                }   }
                else qDebug() << "SubCircuit:"<<m_name<<m_id<< "ERROR Creating Component: "<<type<<uid<<label;
}   }   }   }

void SubCircuit::addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length  )
{
    if( m_initialized && m_pinTunnels.contains( m_id+"-"+id ) )
    {
        updatePin( id, type, label, xpos, ypos, angle, length );
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
        pin->setObjectName( pId );
        pin->setId( pId );
        connect( this, SIGNAL( moved() ), pin, SLOT( isMoved() ), Qt::UniqueConnection );

        if     ( type == "inverted" || type == "in" ) pin->setInverted( true );
        else if( type == "unused"   || type == "nc" )
        {
            pin->setUnused( true );
            if( m_isLS )
            {
                label.remove("!");
                pin->setVisible( false );
                pin->setLabelText( "" );
            }
        }
        tunnel->setRotated( angle >= 180 );      // Our Pins at left side
        if     ( angle == 180) tunnel->setRotation( 0 );
        else if( angle == 90 ) tunnel->setRotation( -90 ); // QGraphicsItem 0º i at right side
        else                   tunnel->setRotation( angle );

        pin->setLength( length );
        pin->setLabelColor( color );
        pin->setLabelText( label.remove(" ") );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );

        m_ePin[pos-1] = pin;
}   }

void SubCircuit::updatePin( QString id, QString type, QString label, int xpos, int ypos, int angle, int length )
{
    Pin* pin = NULL;
    Tunnel* tunnel = m_pinTunnels.value( m_id+"-"+id );
    if( !tunnel )
    {
        qDebug() <<"SubCircuit::updatePin Pin Not Found:"<<id<<type<<label;
        return;
    }
    tunnel->setPos( xpos, ypos );
    tunnel->setRotated( angle >= 180 );      // Our Pins at left side

    if     ( angle == 180) tunnel->setRotation( 0 );
    else if( angle == 90 ) tunnel->setRotation( -90 ); // QGraphicsItem 0º i at right side
    else                   tunnel->setRotation( angle );

    pin = tunnel->getPin();
    type = type.toLower();

    if( m_isLS )
    {
        label.remove("!");
        pin->setLabelColor( QColor( 0, 0, 0 ) );
    }
    else pin->setLabelColor( QColor( 250, 250, 200 ) );

    if( type == "unused"   || type == "nc" )
    {
        pin->setUnused( true );
        if( m_isLS )
        {
            label.remove("!");
            pin->setVisible( false );
            pin->setLabelText( "" );
        }
    }
    pin->setInverted( type == "inverted" || type == "in" );
    pin->setLength( length );
    pin->setLabelText( label.remove(" ") );
    pin->setVisible( true );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );
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
    }   }   }
    if( m_subcType >= Board ){
        for( Component* c : m_compList )
            if( c->isGraphical() ) c->setVisible( !m_isLS );
    }
}

void SubCircuit::remove()
{
    for( Component* comp : m_compList )
    {
        //if( comp->itemType()=="Node" ) continue;
        comp->setParentItem( NULL );
        Circuit::self()->removeComp( comp );
    }
    Component::remove();
}

void SubCircuit::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else{
        event->accept();
        QMenu* menu = new QMenu();
        Component* mainComp = m_mainComponent;
        QString id = m_id;

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
        Component::contextMenu( event, menu );
        menu->deleteLater();
}   }

QString SubCircuit::toString()
{
    QString item = CompBase::toString();
    QString end = " />\n";
    /*if( m_subcType >= Shield )
    {
        QString cp = " circPos=\""+getPropStr( "circPos" )+"\"";
        item.replace( end, cp+end );
    }*/
    if( !m_mainComponent ) return item;

    item.remove( end );
    item += ">";
    item += m_mainComponent->toString().replace( "<item ", "<mainCompProps ");
    item += "</item>\n";

    return item;
}

#include "moc_subcircuit.cpp"
