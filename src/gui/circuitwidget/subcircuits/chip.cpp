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

#define tr(str) simulideTr("Chip",str)

Chip::Chip( QString type, QString id )
    : Component( type, id )
    , eElement( id )
    , m_label( this )
{
    m_id = id;
    QStringList list = id.split("-");
    if( list.size() > 1 ) m_name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"

    m_subcType = None;
    m_isLS = false;
    m_initialized = false;
    m_package  = "";
    m_backPixmap = NULL;
    m_backData   = NULL;
    
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
    m_label.setDefaultTextColor( QColor( 135, 135, 120 ) );
    m_label.setAcceptedMouseButtons( 0 );
    m_label.setRotation(-90 );
    m_label.setVisible( true );
    
    setLabelPos( m_area.x(), m_area.y()-20, 0);
    setShowId( true );
}
Chip::~Chip()
{
    if( m_backPixmap ) delete m_backPixmap;
}

bool Chip::setPropStr( QString prop, QString val )
{
    if( prop =="Logic_Symbol" )
    {
        QString pkgDIP;
        QString pkgLS;
        for( QString key : m_packageList.keys() )
        {
            if( key.endsWith("DIP") ) pkgDIP = key;
            else                      pkgLS  = key;
        }
        if( pkgDIP.isEmpty() ) pkgDIP = m_packageList.first();
        if( pkgLS.isEmpty()  ) pkgLS  = m_packageList.first();

        if( val == "true" ) setPackage( pkgLS );
        else                setPackage( pkgDIP );
        return true;
    }
    return CompBase::setPropStr( prop, val );
}

QString Chip::convertPackage( QString domText ) // Static, converts xml to new format
{
    QString pkg;
    domText.replace("<!--","\n<!--");
    QStringList lines = domText.split("\n");
    for( QString line : lines )
    {
        if( line.isEmpty() ) continue;
        if( line.startsWith("<!") ) continue;
        if( line.startsWith("</") ) continue;
        line.replace("<item itemtype=\"Package\"","Package;");
        line.replace("<packageB","Package;");
        line.replace("&#x3C;", "<").replace("&#x3D;", "=").replace("&#x3E;", ">");
        line.replace("&#x3C" , "<").replace("&#x3D" , "=").replace("&#x3E" , ">");
        line.replace("&#xa;","");
        line.replace("Pin;","\nPin;");
        line.replace("<pin","Pin;");
        line.replace("Pins=","\n");
        line.replace("/>","");
        line.replace("=\"","=");
        line.replace("\"",";");
        line.replace("  ","");
        line.replace(" ;",";");
        line.append("\n");
        pkg.append( line );//
        //qDebug() << line;
    }
    //qDebug() << "-----------------------------------------";
    return pkg;
}

void Chip::setName( QString name )
{
    m_name = name;
    m_label.setPlainText( m_name );
    m_label.adjustSize();
    m_label.setY( m_area.height()/2+m_label.textWidth()/2 );
    m_label.setX( ( m_area.width()/2-m_label.boundingRect().height()/2 ) );
    setflip();
}

void Chip::setPackage( QString package )
{
    m_package = package;

    setLogicSymbol( !package.endsWith("DIP") );

    QString pkgStr = m_packageList.value( package );
    initPackage( pkgStr );
}

void Chip::initPackage( QString pkgStr )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    for( Pin* pin : m_unusedPins ) if( pin ) deletePin( pin );
    m_unusedPins.clear();
    m_ePin.clear();
    m_pin.clear();

    QString embedName;

    QStringList lines = pkgStr.split("\n");
    for( QString line : lines )
    {
        if( line.isEmpty() ) continue;
        if( line.startsWith("Package") )
        {
            QStringList tokens = line.split(";");
            QString type = tokens.takeFirst();

            for( QString prop : tokens )
            {
                while( prop.startsWith(" ") ) prop.remove( 0, 1 );
                QStringList p = prop.split("=");
                if( p.size() != 2 ) continue;

                QString name = p.first().toLower();// Property_name
                QString val  = p.last(); // Property_value

                if     ( name == "width"       ) m_width  = val.toInt();
                else if( name == "height"      ) m_height = val.toInt();
                else if( name == "name"        ) embedName = val;
                else if( name == "background"  ) setBackground( val );
                else if( name == "bckgnddata"  ) setBckGndData( val );
                else if( name == "logic_symbol") setLogicSymbol( val == "true" );
            }
        }
        else if( line.startsWith("Pin") ) setPinStr( line );
    }
    m_initialized = true;
    m_area = QRect( 0, 0, 8*m_width, 8*m_height );
    if( embedName == "Package" ) embedName = m_name;
    setName( embedName );
    moveSignal();
    update();
    Circuit::self()->update();
}

void Chip::setPinStr( QString pin )
{
    //qDebug() << "Chip::setPinStr" << pin;
    QStringList tokens = pin.split(";");

    int length = 8;
    int xpos   = -m_width/2-length;
    int ypos   = 8;
    int angle  = 0;
    int space  = 0;
    QString id;
    QString label;
    QString type;

    for( QString token : tokens )
    {
        while( token.startsWith(" ") ) token.remove( 0, 1 );
        QStringList p = token.split("=");
        if( p.size() != 2 ) continue;

        QString prop = p.first();
        QString val  = p.last();
        if     ( prop == "xpos"  ) xpos   = val.toInt();
        else if( prop == "ypos"  ) ypos   = val.toInt();
        else if( prop == "angle" ) angle  = val.toInt();
        else if( prop == "length") length = val.toInt();
        else if( prop == "space" ) space  = val.toInt();
        else if( prop == "id"    ) id     = val;
        else if( prop == "label" ) label  = val;
        else if( prop == "type"  ) type   = val;
    }
    addNewPin( id, type, label, 0, xpos, ypos, angle, length, space );
}

void Chip::addNewPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length, int space )
{
    if( type == "unused" || type == "nc" )
    {
        Pin* pin = new Pin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this ); // pos in package starts at 1

        if( m_isLS ) pin->setVisible( false );
        else         pin->setLabelText( label );
        pin->setSpace( space );
        pin->setUnused( true ); // Chip::addPin is only for unused Pins
        pin->setLength( length );
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

        m_unusedPins.append( pin );
    }else{
        Pin* pin = addPin( id, type, label, pos, xpos, ypos, angle, length, space );
        m_ePin.emplace_back( pin );
        m_pin.emplace_back( pin );
    }
}

void Chip::setLogicSymbol( bool ls )
{
    m_isLS = ls;
    if( m_isLS ) m_color = m_lsColor;
    else         m_color = m_icColor;

    QColor labelColor = QColor( 0, 0, 0 );

    if( ls ) m_color = m_lsColor;
    else{
        m_color = m_icColor;
        labelColor = QColor( 250, 250, 200 );
    }
    for( Pin* pin : m_pin ) pin->setLabelColor( labelColor );

    Circuit::self()->update();
}

void Chip::setBckGndData( QString data )
{
    QByteArray ba;
    bool ok;
    for( int i=0; i<data.size(); i+=2 )
    {
        QString ch = data.mid( i, 2 );
        ba.append( ch.toInt( &ok, 16 ) );
    }

    if( !m_backPixmap ) m_backPixmap = new QPixmap();
    m_backPixmap->loadFromData( ba );
    update();
}

void Chip::setBackground( QString bck )
{
    m_background = bck;

    if( m_backPixmap )
    {
        delete m_backPixmap;
        m_backPixmap = NULL;
    }
    if( bck.startsWith("color") )
    {
        bck.remove("color").remove("(").remove(")").remove(" ");
        QStringList rgb = bck.split(",");
        if( rgb.size() < 3 ) return;

        m_color = QColor( rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt() );
    }
    else if( bck != "" ){
        QDir dir = QFileInfo( m_dataFile ).absoluteDir();
        QString pixmapPath = dir.absoluteFilePath( bck );  // Image in subcircuit folder

        if( !QFile::exists( pixmapPath ) ){
            dir = QFileInfo( Circuit::self()->getFilePath() ).absoluteDir();
            pixmapPath = dir.absoluteFilePath( bck );    // Image in circuit/data folder
        }
        if( !QFile::exists( pixmapPath ) ) pixmapPath = MainWindow::self()->getDataFilePath("images/"+bck );
        if( QFile::exists( pixmapPath ) ) m_backPixmap = new QPixmap( pixmapPath );
    }
    update();
}

void Chip::setflip()
{
    Component::setflip();
    m_label.setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
    int xDelta = m_Hflip*m_label.boundingRect().height()/2;
    int yDelta = m_Vflip*m_label.textWidth()/2;
    m_label.setY( m_area.height()/2+yDelta );
    m_label.setX( ( m_area.width()/2-xDelta ) );
}

void Chip::findHelp()
{
    QString helpFile = changeExt( m_dataFile, "txt" );
    if( QFileInfo::exists( helpFile ) ) m_help = fileToString( helpFile, "Chip::findHelp" );
    else                                m_help = MainWindow::self()->getHelp( m_name, false );
}

void Chip::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    if( m_backPixmap ) p->drawPixmap( QRect(m_area.x(), m_area.y(), m_width*8, m_height*8), *m_backPixmap );
    else{
        p->drawRoundedRect( m_area, 1, 1);
        if( m_backData  )
        {
            double w = m_backData->size();
            double h = m_backData->at(0).size();

            QImage img( w*3, h*3, QImage::Format_RGB32 );
            QPainter painter;
            painter.begin( &img );

            for( int col=0; col<w; col++ )
            {
                int x = col*3;
                for( int y=0; y<h; y++ )
                    painter.fillRect( QRectF( x, y*3, 3, 3 ), QColor(m_backData->at(col).at(y) ) );
            }
            painter.end();
            p->drawImage( m_area, img );
        }
        else if( !m_isLS && m_background.isEmpty() )
        {
            p->setPen( QColor( 170, 170, 150 ) );
            if( m_width == m_height ) p->drawEllipse( 4, 4, 4, 4);
            else                      p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
        }
    }
    Component::paintSelected( p );
}
