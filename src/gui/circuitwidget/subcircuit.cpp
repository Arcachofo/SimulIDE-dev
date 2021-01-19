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
#include "circuit.h"
#include "tunnel.h"
#include "utils.h"
#include "node.h"
#include "simuapi_apppath.h"

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
    QString compName = m_id.split("-").first(); // for example: "atmega328-1" to: "atmega328"

    m_icColor = QColor( 20, 30, 60 );

    QFont f = QFontDatabase::systemFont( QFontDatabase::FixedFont );
    f.setFamily("Monospace");
    f.setPixelSize(5);
    f.setLetterSpacing( QFont::PercentageSpacing, 120 );
    m_valLabel->setFont( f );
    m_valLabel->setPlainText( compName );
    m_valLabel->setDefaultTextColor( QColor( 110, 110, 110 ) );
    m_valLabel->setAcceptedMouseButtons( 0 );
    setValLabelX( m_area.width()/2-3 );
    setValLabelY( m_area.height()/2+m_valLabel->textWidth()/2 );
    setValLabRot(-90 );
    setValLabelPos();
    setShowVal( true );


    QString dataFile = ComponentSelector::self()->getXmlFile( compName );

    m_mainComponent = 0l;
    //qDebug()<<"SubCircuit::SubCircuit"<<compName<<dataFile;

    if( dataFile == "" )
    {
          MessageBoxNB( "SubCircuit::SubCircuit", "                               \n"+
                    tr( "There are no data files for " )+compName+"    ");
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

            if( element.attribute("name") == compName )
            {
                QDir dataDir( dataFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name

                QString subcFile = "";
                QString path = "";

                if( folder != "" )
                {
                    path = folder+"/"+compName+"/"+compName;
                    m_pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".simu" );
                }
                if( element.hasAttribute( "folder") )
                {
                    path = element.attribute( "folder" )+"/"+compName+"/"+compName;
                    m_pkgeFile = dataDir.filePath( path+".package" );
                    subcFile = dataDir.filePath( path+".simu" );
                }
                if( element.hasAttribute( "package") )
                    m_pkgeFile = dataDir.filePath( element.attribute( "package" ) );

                if( !m_pkgeFile.endsWith( ".package" ) ) m_pkgeFile += ".package" ;
                
                if( element.hasAttribute( "subcircuit") )
                    subcFile = dataDir.filePath( element.attribute( "subcircuit" ) );

                if( !subcFile.endsWith( ".simu" ) ) subcFile += ".simu" ;

                //qDebug() << "SubCircuit::SubCircuit"<<m_pkgeFile <<m_subcFile ;
                loadSubCircuit( subcFile );

                if( m_mainComponent )  // Example MCU in subcircuit needs to know where subcircuit is.
                {
                    dataDir.setPath( subcFile );
                    dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                    m_mainComponent->setSubcDir( dataDir.absolutePath() );
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
            comp->setParentItem( 0l );
            Circuit::self()->removeComp( comp );
        }
    }
    else    initChip();
}
SubCircuit::~SubCircuit(){}

QList<propGroup_t> SubCircuit::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Logic_Symbol", tr("Logic Symbol"),""} );
    return {mainGroup};
}

void SubCircuit::loadSubCircuit( QString fileName )
{
    QFile file( fileName );

    //qDebug() << "Circuit::loadCircuit"<<m_filePath;

    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
        QMessageBox::warning(0l, "SubCircuit::loadSubCircuit",
        tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        m_error = 32;
        return;
    }
    QDomDocument domDoc;

    if( !domDoc.setContent(&file) )
    {
        QMessageBox::warning( 0l, "SubCircuit::loadSubCircuit",
        tr("Cannot set file %1\nto DomDocument").arg(fileName));
        file.close();
        m_error = 33;
        return;
    }
    file.close();

    loadDomDoc( &domDoc );
}

void SubCircuit::loadDomDoc( QDomDocument* doc )
{
    Circuit* circ = Circuit::self();

    QHash<QString, eNode*> nodMap;

    QDomNode node = doc->documentElement().firstChild();

    while( !node.isNull() )
    {
        QDomElement   element = node.toElement();
        const QString tagName = element.tagName();

        if( tagName == "item" )
        {
            QString objNam = element.attribute( "objectName"  ); // Data in simu file
            QString type   = element.attribute( "itemtype"  );
            QString id     = objNam.split("-").first()+"-"+circ->newSceneId(); // Create new id

            element.setAttribute( "objectName", id  );

            if( type == "Connector" )
            {
                Pin* startpin  = circ->getConPin( element.attribute( "startpinid" ) );
                Pin* endpin    = circ->getConPin( element.attribute( "endpinid" ) );

                if( startpin && endpin )    // Create Connector
                {
                    Connector* con  = new Connector( this, type, id, startpin, endpin );

                    element.setAttribute( "startpinid", startpin->objectName() );
                    element.setAttribute(   "endpinid", endpin->objectName() );

                    circ->loadProperties( element, con );

                    QString enodeId = element.attribute( "enodeid" );
                    eNode*  enode   = nodMap[enodeId];

                    if( !enode )          // Create eNode and add to enodList
                    {
                        enode = new eNode( "Circ_eNode-"+circ->newSceneId() );
                        nodMap[enodeId] = enode;
                    }
                    con->setEnode( enode );
                    con->setVisib( false );
                    con->setHidden( true, true );
                    con->setParentItem( this );
                    con->setPos( 0, 0 );
                    circ->conList()->removeOne( con );
                }
                else // Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null startpin in " << objNam;
                    if( !endpin )   qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null endpin in "   << objNam;
                }
            }
            else if( type == "Package" ) { ; }
            else
            {
                Component* comp = 0l;
                if( type == "Node" )
                {
                    comp = new Node( this, type, id );
                    circ->m_idMap[objNam] = id; // Map simu id to new id
                }
                else
                {
                    if( objNam == "" ) objNam = id;
                    Circuit::self()->m_idMap[objNam] = id; // Map simu id to new id
                    comp = circ->createItem( type, id, objNam );
                }

                if( comp )
                {
                    circ->loadProperties( element, comp );
                    comp->setParentItem( this );
                    QPointF pos = comp->boardPos();
                    if( pos == QPointF( -1e6, -1e6 ) ) pos = QPointF( 0, 0 );
                    comp->moveTo( pos );
                    comp->setRotation( comp->boardRot() );
                    comp->setHidden( true, true );
                    circ->compList()->removeOne( comp );
                    m_compList.append( comp );

                    if( comp->isMainComp() ) m_mainComponent = comp; // This component will add it's Context Menu

                    if( type == "Tunnel" ) // Make Tunnel names unique for this subcircuit
                    {
                        Tunnel* tunnel = static_cast<Tunnel*>( comp );
                        tunnel->setUid( tunnel->name() );
                        tunnel->setName( m_id+"-"+tunnel->name() );
                    }
                }
                else qDebug() << " ERROR Creating Component: "<< type << id;
            }
        }
        node = node.nextSibling();
    }
}

void SubCircuit::addPin(QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length  )
{
    if( m_initialized && m_tunnelList.contains( m_id+"-"+id ) )
    {
        updatePin( id, type, label, pos, xpos, ypos, angle, length );
    }
    else
    {
        //qDebug() << "SubCircuit::addPin"<<id<<label;
        QColor color = Qt::black;
        if( !m_isLS ) color = QColor( 250, 250, 200 );

        Tunnel* tunnel = new Tunnel( this, "Tunnel", m_id+"-"+id );
        Circuit::self()->compList()->removeOne( tunnel );
        m_compList.append( tunnel );

        tunnel->setParentItem( this );
        tunnel->setAcceptedMouseButtons( Qt::NoButton );
        tunnel->setShowId( false );
        tunnel->setName( m_id+"-"+id ); // Make tunel name unique for this component
        tunnel->setPos( xpos, ypos );
        tunnel->setPacked( true );
        m_tunnelList.insert( m_id+"-"+id, tunnel );

        Pin* pin = tunnel->getPin();
        pin->setObjectName( m_id+"-"+id );
        pin->setId( m_id+"-"+id );
        pin->setLabelColor( color );
        pin->setLabelText( label );
        connect( this, SIGNAL( moved() ),
                  pin, SLOT( isMoved() ), Qt::UniqueConnection );

        if     ( type == "inverted" ) pin->setInverted( true );
        else if( type == "unused" )   pin->setUnused( true );
        else if( type == "null" )
        {
            pin->setVisible( false );
            pin->setLabelText( "" );
        }
        if     ( angle == 90 )  tunnel->setRotation( angle );
        else if( angle >= 180 ) tunnel->setRotated( true );
        if( angle == 270 ) tunnel->setRotation( angle );

        pin->setLength( length );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, (length<8) );

        m_ePin[pos-1] = pin;
    }
}

void SubCircuit::updatePin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length )
{
    Pin* pin = NULL;
    Tunnel* tunnel = m_tunnelList.value( m_id+"-"+id );
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
        for( QString tNam : m_tunnelList.keys() )
        {
            Tunnel* tunnel = m_tunnelList.value( tNam );
            Pin* pin = tunnel->getPin();
            if( pin->unused() )
            {
                pin->setVisible( false );
                pin->setLabelText( "" );
            }
        }
    }
    setValLabelX( m_area.width()/2-3 );
    setValLabelY( m_area.height()/2+m_valLabel->textWidth()/2 );
    setValLabRot(-90 );
    setValLabelPos();
}

void SubCircuit::remove()
{
    for( Component* comp : m_compList )
    {
        comp->setParentItem( 0l );
        Circuit::self()->removeComp( comp );
    }
    Component::remove();
}

void SubCircuit::slotProperties()
{
    Component::slotProperties();

    /*if( m_properties ) m_propertiesW->show();
    else
    {
        Component::slotProperties();
        if( m_mainComponent )
            m_propertiesW->properties()->addObject( m_mainComponent );
    }*/
}

void SubCircuit::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        if( m_mainComponent )
        {
            menu->addSection( m_mainComponent->itemType() );
            m_mainComponent->contextMenu( event, menu );
            menu->addSection( "Subcircuit" );
        }
        Component::contextMenu( event, menu );
        menu->deleteLater();
    }
}

#include "moc_subcircuit.cpp"
