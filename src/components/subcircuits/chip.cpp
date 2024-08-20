/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "chip.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "simulator.h"
#include "connector.h"
#include "circuit.h"
#include "utils.h"
#include "pin.h"

QString Chip::s_subcType = "None";

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
        if( m_packageList.isEmpty() ) setLogicSymbol( val == "true" );
        else{
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
        }
        return true;
    }
    return Component::setPropStr( prop, val );
}

QMap<QString, QString> Chip::getPackages( QString compFile ) // Static
{
    QMap<QString, QString> packageList;

    QString doc = fileToString( compFile, "Chip::getPackages");

    QVector<QStringRef> docLines = doc.splitRef("\n");

    for( QStringRef line : docLines )
    {
        if( !line.startsWith("<item") ) continue;

        QVector<propStr_t> properties = parseXmlProps( line );
        propStr_t itemType = properties.takeFirst();
        if( itemType.name != "itemtype") continue;
        if( itemType.value != "Package") break;    // All packages processed

        bool addPackage = false;
        QString pkgName;
        QString pkgStr = "Package; ";

        for( propStr_t prop : properties )
        {
            QString propName  = prop.name.toString();
            QString propValue = prop.value.toString();

            if     ( propName == "SubcType" ) { if( propValue != "None" ) s_subcType = propValue; } // Only for Subcircuits
            else if( propName == "label"    ) pkgName = propValue;
            //else if( propName == "Logic_Symbol") ls = ( propValue == "true");

            if( propName == "Pins"){
                propValue.replace("&#xa;","\n");
                if( propValue.contains("&") ) propValue = cleanPinName( propValue );
                pkgStr += "\n"+propValue;
                addPackage = true;         // Package contains Pin info (new circuits)
            }
            else pkgStr += propName+"="+propValue+"; ";
        }
        if( addPackage && !pkgName.isEmpty() ) packageList[pkgName] = pkgStr;
    }
    return packageList;
}

QString Chip::convertPackage( QString pkgText ) // Static, converts xml to new format
{
    QString pkgStr;
    QVector<QStringRef> docLines = pkgText.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.startsWith("<!") ) continue;
        if( line.startsWith("</") ) continue;
        if( line.isEmpty() ) continue;

        QVector<propStr_t> properties = parseXmlProps( line );

        if( line.startsWith("<package") )
        {
            pkgStr += "Package; ";

            for( propStr_t prop : properties )
            {
                QString propName  = prop.name.toString();
                QString propValue = prop.value.toString();
                if( propName == "type" ) s_subcType = propValue.remove("subc");
                pkgStr += propName+"="+propValue+"; ";
            }
        }else if( !properties.isEmpty() )
        {
            pkgStr += "Pin; ";
            for( propStr_t prop : properties )
            {
                QString value = prop.value.toString();
                if( value.contains("&") ) value = cleanPinName( value );
                pkgStr += prop.name.toString()+"="+value+"; ";
            }
        }
        pkgStr += "\n";
    }
    //qDebug() << pkgStr;

    return pkgStr;
}

QString Chip::cleanPinName( QString name )
{
    name.replace("&#x3D;", "=").replace("&#x3C;", "<").replace("&#x3E;", ">")
        .replace("&#x3D" , "=").replace("&#x3C" , "<").replace("&#x3E" , ">")
        .replace("&lt;", "<") ;
    return name;
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
    if( !m_packageList.contains( package) ) package = m_packageList.keys().first();
    if( m_package == package ) return;

    m_package = package;

    m_isLS = !package.endsWith("DIP");

    QString pkgStr = m_packageList.value( package );
    initPackage( pkgStr );
    setLogicSymbol( m_isLS );
}

void Chip::initPackage( QString pkgStr )
{
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    m_customColor = false;
    m_tempPins.clear();
    for( Pin* pin : m_ncPins ) m_tempPins.append( pin );
    for( Pin* pin : m_pin    ) m_tempPins.append( pin );
    m_ncPins.clear();
    m_ePin.clear();
    m_pin.clear();

    QString embedName;

    QVector<QStringRef> docLines = pkgStr.splitRef("\n");
    for( QStringRef line : docLines )
    {
        if( line.isEmpty() ) continue;

        QVector<propStr_t> properties = parseProps( line );
        if( properties.isEmpty() ) break;

        QStringRef item = properties.takeFirst().name;
        if( item == "Package" )
        {
            for( propStr_t property : properties )
            {
                QString   name = property.name.toString().toLower();  // Property name
                QStringRef val = property.value;                      // Property value

                if     ( name == "width"     ) m_width  = val.split(" ").first().toInt();
                else if( name == "height"    ) m_height = val.split(" ").first().toInt();
                else if( name == "name"      ) embedName = val.toString();
                else if( name == "background") setBackground( val.toString() );
                else if( name == "bckgnddata") setBckGndData( val.toString() );
                else if( name == "logic_symbol") m_isLS = ( val == "true" );
            }
        }
        else if( item == "Pin" ) setPinStr( properties );
    }

    for( Pin* pin : m_tempPins ) // Pins present in previous package and not present in this one
    {
        pin->setVisible( false );
        pin->setLabelText("");
    }
    m_initialized = true;
    m_area = QRect( 0, 0, 8*m_width, 8*m_height );
    if( m_subcType >= Board ) setTransformOriginPoint( toGrid( boundingRect().center()) );

    if( embedName == "Package" ) embedName = m_name;
    setName( embedName );
    moveSignal();
    update();
    Circuit::self()->update();
}

void Chip::setPinStr( QVector<propStr_t> properties )
{
    int length = 8;
    int xpos   = -m_width/2-length;
    int ypos   = 8;
    int angle  = 0;
    int space  = 0;
    QString id;
    QString label;
    QString type;

    for( propStr_t property : properties )
    {
        QStringRef name = property.name;  // Property_name
        QStringRef val  = property.value; // Property_value

        if     ( name == "xpos"  ) xpos   = val.toInt();
        else if( name == "ypos"  ) ypos   = val.toInt();
        else if( name == "angle" ) angle  = val.toInt();
        else if( name == "length") length = val.toInt();
        else if( name == "space" ) space  = val.toInt();
        else if( name == "id"    ) id     = val.toString();
        else if( name == "label" ) label  = val.toString();
        else if( name == "type"  ) type   = val.toString();
    }
    addNewPin( id, type, label, 0, xpos, ypos, angle, length, space );
}

void Chip::addNewPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length, int space )
{
    Pin* pin = nullptr;

    if( m_pinMap.contains( id ) )
    {
        pin = updatePin( id, type, label, xpos, ypos, angle, length, space );
    }
    if( !pin ){
        if( type == "unused" || type == "nc" )
        {
            if( m_pinMap.contains( id ) )
            {
                pin = m_pinMap.value( id );
                pin->setPos( xpos, ypos );
                pin->setPinAngle( angle );
            }
            else pin = new Pin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this ); // pos in package starts at 1

            pin->setVisible( true );
            pin->setLabelText( label );
            pin->setSpace( space );
            pin->setUnused( true ); // Chip::addPin is only for unused Pins
            pin->setLength( length );
            pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
            pin->isMoved();
        }
        else pin = addPin( id, type, label, pos, xpos, ypos, angle, length, space );

        if( !m_pinMap.contains( id ) ) m_pinMap.insert( id, pin );
    }
    if( !pin ) return;

    if( pin->unused() ) m_ncPins.append( pin );
    else{
        m_ePin.emplace_back( pin );
        m_pin.emplace_back( pin );
    }
    m_tempPins.removeOne( pin );
}

void Chip::setLogicSymbol( bool ls )
{
    m_isLS = ls;
    QColor labelColor = QColor( 0, 0, 0 );

    if( ls ){
        if( !m_customColor ) m_color = m_lsColor;
    }else{
        if( !m_customColor ) m_color = m_icColor;
        labelColor = QColor( 250, 250, 200 );
    }
    for( Pin* pin : m_pin ) pin->setLabelColor( labelColor );

    Circuit::self()->update();
}

void Chip::setBckGndData( QString data )
{
    if( m_backPixmap )
    {
        delete m_backPixmap;
        m_backPixmap = nullptr;
    }
    if( data.isEmpty() ) return;

    m_backPixmap = new QPixmap();

    QByteArray ba;
    bool ok;
    for( int i=0; i<data.size(); i+=2 )
    {
        QString ch = data.mid( i, 2 );
        ba.append( ch.toInt( &ok, 16 ) );
    }
    m_backPixmap->loadFromData( ba );
    update();
}

void Chip::setBackground( QString bck )
{
    m_background = bck;

    if( bck.startsWith("color") )
    {
        bck.remove("color").remove("(").remove(")").remove(" ");
        QStringList rgb = bck.split(",");
        if( rgb.size() < 3 ) return;

        m_color = QColor( rgb.at(0).toInt(), rgb.at(1).toInt(), rgb.at(2).toInt() );
        m_customColor = true;
    }
    else if( bck != "" )
    {
        QString pixmapPath = MainWindow::self()->getCircFilePath( bck ); // Image in circuit/data folder

        if( !QFile::exists( pixmapPath ) ){
            QDir dir = QFileInfo( m_dataFile ).absoluteDir();
            pixmapPath = dir.absoluteFilePath( bck );              // Image in subcircuit folder
        }
        if( !QFile::exists( pixmapPath ) )                    // Image in user/data/images or simulide/data/images
            pixmapPath = MainWindow::self()->getDataFilePath("images/"+bck );

        if( QFile::exists( pixmapPath ) )
        {
            if( !m_backPixmap ) m_backPixmap = new QPixmap();
            m_backPixmap->load( pixmapPath );
        }
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
        else if( m_subcType < Board && !m_isLS /*&& m_background.isEmpty()*/ )
        {
            p->setPen( QColor( 170, 170, 150 ) );
            if( m_width == m_height ) p->drawEllipse( 4, 4, 4, 4);
            else                      p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
        }
    }
    Component::paintSelected( p );
}
