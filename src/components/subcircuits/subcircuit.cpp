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
#include "utils.h"
#include "linker.h"

#include "logicsubc.h"
#include "board.h"
#include "shield.h"
#include "module.h"

#include "stringprop.h"

#define tr(str) simulideTr("SubCircuit",str)

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

    if( Circuit::self()->getSubcircuit() )
    {
        int rev = Circuit::self()->circuitRev();
        if( rev >= 2220 ){ if( name.contains("@") ) list = name.split("@");}
        else if( name.contains("_") ) list = name.split("_");

        if( list.size() > 1 )  // Subcircuit inside Subcircuit: 1_74HC00 to 74HC00
        {
            QString n = list.first();
            bool ok = false;
            n.toInt(&ok);
            if( ok ) name = list.at( 1 );
        }
    }

    QMap<QString, QString> packageList;
    QString subcTyp = "None";
    QString subcDoc;
    QString pkgeFile;
    QString subcFile;
    QString dataFile;

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
                return nullptr;
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
        return nullptr;
    }

    SubCircuit* subcircuit = nullptr;
    if     ( subcTyp == "Logic"  ) subcircuit = new LogicSubc( type, id );
    else if( subcTyp == "Board"  ) subcircuit = new BoardSubc( type, id );
    else if( subcTyp == "Shield" ) subcircuit = new ShieldSubc( type, id );
    else if( subcTyp == "Module" ) subcircuit = new ModuleSubc( type, id );
    else                           subcircuit = new SubCircuit( type, id );

    if( m_error != 0 )
    {
        subcircuit->remove();
        m_error = 0;
        return nullptr;
    }else{
        Circuit::self()->setSubcircuit( subcircuit );

        QStringList pkges = packageList.keys();
        subcircuit->m_packageList = packageList;
        subcircuit->m_dataFile = subcFile;

        if( packageList.size() > 1 ) // Add package list if there is more than 1 to choose
        subcircuit->addProperty( tr("Main"),
        new StrProp <SubCircuit>("Package", tr("Package"),""
                                , subcircuit, &SubCircuit::package, &SubCircuit::setPackage,0,"enum" ));

        subcircuit->setPackage( pkges.first() );
        if( m_error == 0 ) subcircuit->loadSubCircuitFile( subcFile );
    }
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subcircuit );
        delete subcircuit;
        m_error = 0;
        return nullptr;
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
          , EmbedCircuit( m_name, id, this )
{
    m_lsColor = QColor( 235, 240, 255 );
    m_icColor = QColor( 20, 30, 60 );

    addPropGroup( { tr("Main"), {},0} );
}
SubCircuit::~SubCircuit(){}

Pin* SubCircuit::addPin( QString id, QString type, QString label, int, int xpos, int ypos, int angle, int length, int space )
{
    if( m_initialized && m_pinTunnels.contains( m_ecId+"-"+id ) )
    {
        return EmbedCircuit::updatePin( id, type, label, xpos, ypos, angle, length );
    }else
        return EmbedCircuit::addPin( id, type, label, 0, xpos, ypos, angle, length, space );
}

Pin* SubCircuit::updatePin( QString id, QString type, QString label, int xpos, int ypos, int angle, int length, int space )
{
    return EmbedCircuit::updatePin( id, type, label, xpos, ypos, angle, length, space );
}

void SubCircuit::setLogicSymbol( bool ls )
{
    Chip::setLogicSymbol( ls );

    /*if( m_isLS )
    {
        for( QString tNam : m_pinTunnels.keys() )   // Don't show unused Pins in LS
        {
            Tunnel* tunnel = m_pinTunnels.value( tNam );
            Pin* pin = tunnel->getPin();
            if( pin->unused() ) { pin->setVisible( false ); pin->setLabelText(""); }
        }
        if( m_backPixmap )    // No background image in LS
        {
            delete m_backPixmap;
            m_backPixmap = nullptr;
        }
    }*/
    for( Component* comp : m_compList ) // Don't show graphical components in LS if Board
    {
        if( !comp->isGraphical() ) continue;
        if( this->isBoard() )
        {
            comp->setVisible( !m_isLS && comp->boardPos() != QPointF(-1e6,-1e6 ) );
        }
        else if( m_isLS )  comp->setVisible( false );
    }
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

QString SubCircuit::toString()
{
    QString item = CompBase::toString();
    item.append( EmbedCircuit::toString() );
    return item;
}
