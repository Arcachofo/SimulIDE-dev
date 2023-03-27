/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDomDocument>

#include "chip.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "simulator.h"
#include "connector.h"
#include "circuit.h"
#include "utils.h"
#include "pin.h"

Chip::Chip( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
    , m_label( this )
{
    m_id = id;
    QStringList list = id.split("-");
    if( list.size() > 1 ) m_name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"

    m_enumUids = QStringList()
        << "None"
        << "Logic"
        << "Board"
        << "Shield"
        << "Module";

    m_enumNames = QStringList()
        << tr("None")
        << tr("Logic")
        << tr("Board")
        << tr("Shield")
        << tr("Module");

    m_subcType = None;
    m_isLS = false;
    m_initialized = false;
    m_pkgeFile = "";
    m_BackPixmap = NULL;
    
    m_lsColor = QColor( 255, 255, 255 );
    m_icColor = QColor( 50, 50, 70 );

    QFont f;
    f.setFamily("Ubuntu Mono");
    f.setWeight( 65 );
#ifdef Q_OS_UNIX
    f.setLetterSpacing( QFont::PercentageSpacing, 120 );
#else
    //f.setLetterSpacing( QFont::AbsoluteSpacing, -1 );
    f.setWeight( 100 );
    //f.setStretch( 99 );
#endif
    f.setPixelSize(5);

    m_label.setFont( f );
    m_label.setDefaultTextColor( QColor( 125, 125, 110 ) );
    m_label.setAcceptedMouseButtons( 0 );
    m_label.setRotation( -90 );
    m_label.setVisible( true );
    
    setLabelPos( m_area.x(), m_area.y()-20, 0);
    setShowId( true );
}
Chip::~Chip()
{
    if( m_BackPixmap ) delete m_BackPixmap;
}

void Chip::initChip()
{
    m_error = 0;

    QDir circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
    QString fileNameAbs = circuitDir.absoluteFilePath( m_pkgeFile );

    QFile pfile( fileNameAbs );
    if( !pfile.exists() )   // Check if package file exist, if not try LS or no LS
    {
        if( m_initialized ) { m_error = 1; return; }
        if     ( m_pkgeFile.endsWith("_LS.package")) m_pkgeFile.replace( "_LS.package", ".package" );
        else if( m_pkgeFile.endsWith(".package"))    m_pkgeFile.replace( ".package", "_LS.package" );
        else{
            m_error = 1;
            qDebug() << "Chip::initChip: No package files found.\n";
        }
        fileNameAbs = circuitDir.absoluteFilePath( m_pkgeFile );
    }
    QDomDocument domDoc = fileToDomDoc( fileNameAbs, "Chip::initChip" );
    QDomElement   root  = domDoc.documentElement();

    if( root.tagName() == "packageB" )
    {
        if( m_pkgeFile.endsWith( "_LS.package" )) m_isLS = true;
        else                                      m_isLS = false;

        if( m_isLS ) m_color = m_lsColor;
        else         m_color = m_icColor;

        m_width   = root.attribute( "width" ).toInt();
        m_height  = root.attribute( "height" ).toInt();
        m_area = QRect( 0, 0, 8*m_width, 8*m_height );

        for( Pin* pin : m_unusedPins ) deletePin( pin );
        m_unusedPins.clear();
        m_ePin.clear();
        m_pin.clear();

        if( root.hasAttribute("type") ) setSubcTypeStr( root.attribute("type") );
        if( root.hasAttribute("background") ) setBackground( root.attribute("background") );
        if( m_subcType >= Board ) setTransformOriginPoint( toGrid( boundingRect().center()) );
        if( root.hasAttribute("name"))
        {
            QString name = root.attribute("name");
            if( name.toLower() != "package" ) m_name = name;
        }
        initPackage( root );
        setName( m_name );
    }else{
        qDebug() <<"Chip::initChip"<<"Error: Not valid Package file:\n"<< m_pkgeFile;
        m_error = 3;
        return;
    }
    m_initialized = true;
}

void Chip::setName( QString name )
{
    m_name = name;
    m_label.setPlainText( m_name );
    m_label.adjustSize();
    m_label.setY( m_area.height()/2+m_label.textWidth()/2 );
    m_label.setX( ( m_area.width()/2-m_label.boundingRect().height()/2 ) );
}

void Chip::initPackage( QDomElement root )
{
    int chipPos = 0;
    QDomNode node = root.firstChild();
    while( !node.isNull() )
    {
        QDomElement element = node.toElement();
        if( element.tagName() == "pin" )
        {
            QString type  = element.attribute( "type" );
            QString label = element.attribute( "label" );
            QString id    = element.attribute( "id" ).remove(" ");

            int xpos   = element.attribute( "xpos" ).toInt();
            int ypos   = element.attribute( "ypos" ).toInt();
            int angle  = element.attribute( "angle" ).toInt();
            int length = element.attribute( "length" ).toInt();

            chipPos++;
            addNewPin( id, type, label, chipPos, xpos, ypos, angle, length );
        }
        node = node.nextSibling();
    }
    update();
}

void Chip::addNewPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length )
{
    if( type == "unused" || type == "nc" )
    {
        Pin* pin = new Pin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this ); // pos in package starts at 1

        pin->setUnused( true ); // Chip::addPin is only for unused Pins
        if( m_isLS )
        {
            pin->setVisible( false );
            label = "";
        }
        pin->setLabelText( label );
        pin->setLength( length );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

        m_unusedPins.append( pin );
    }
    else{
        Pin* pin = addPin( id, type, label, pos, xpos, ypos, angle, length );
        m_ePin.emplace_back( pin );
        m_pin.emplace_back( pin );
    }
}

void Chip::deletePin( Pin* pin )
{
    if( !pin ) return;

    pin->removeConnector();
    m_signalPin.removeAll( pin );
    delete pin;
}

void Chip::setLogicSymbol( bool ls )
{
    if( m_initialized && (m_isLS == ls) ) return;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    /// Undo/Redo stack for Properties ??
    /// Circuit::self()->addCompState( this, "Logic_Symbol" );
    
    if(  ls && m_pkgeFile.endsWith(".package"))    m_pkgeFile.replace( ".package", "_LS.package" );
    if( !ls && m_pkgeFile.endsWith("_LS.package")) m_pkgeFile.replace( "_LS.package", ".package" );

    m_error = 0;
    Chip::initChip();
    
    if( m_error == 0 ) Circuit::self()->update();
    /// else               Circuit::self()->unSaveState();
}

void Chip::setBackground( QString bck )
{
    m_background = bck;

    if( m_BackPixmap )
    {
        delete m_BackPixmap;
        m_BackPixmap = NULL;
    }
    if( bck.startsWith("color") )
    {
        bck.remove("color").remove("(").remove(")").remove(" ");
        QStringList rgb = bck.split(",");
        if( rgb.size() < 3 ) return;

        m_color = QColor( rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt() );
    }
    else if( bck != "" ){
        QString pixmapPath = MainWindow::self()->getFilePath("data/images")+"/"+bck;
        if( QFile::exists( pixmapPath ) )              // Image in simulide data folder
            m_BackPixmap = new QPixmap( pixmapPath );
        else                                           // Image in Circuit data folder
        {
            QDir    circuitDir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
            QString pixmapPath = circuitDir.absoluteFilePath( "data/"+m_name+"/"+bck );
            if( QFile::exists( pixmapPath ) ) m_BackPixmap = new QPixmap( pixmapPath );
        }
    }
    update();
}

void Chip::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    if( m_BackPixmap ) p->drawPixmap( m_area.x(), m_area.y(),m_width*8, m_height*8, *m_BackPixmap );
    else{
        p->drawRoundedRect( m_area, 1, 1);
        if( !m_isLS && m_background.isEmpty() )
        {
            p->setPen( QColor( 170, 170, 150 ) );
            if( m_width == m_height ) p->drawEllipse( 4, 4, 4, 4);
            else                      p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
}   }   }
