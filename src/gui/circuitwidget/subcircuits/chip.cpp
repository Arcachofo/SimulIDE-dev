/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "chip.h"
#include "circuitwidget.h"
#include "mainwindow.h"
#include "simulator.h"
#include "connector.h"
#include "circuit.h"
#include "utils.h"
#include "pin.h"

QStringList Chip::m_subcTypes = {"None","Logic","Board","Shield","Module"};

Chip::Chip( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
    , m_label( this )
{
    m_id = id;
    QStringList list = id.split("-");
    if( list.size() > 1 ) m_name = list.at( list.size()-2 ); // for example: "atmega328-1" to: "atmega328"

    m_subcType = None;
    m_numpins = 0;
    m_isLS = false;
    m_initialized = false;
    m_pkgeFile = "";
    m_BackPixmap = NULL;
    
    m_lsColor = QColor( 255, 255, 255 );
    m_icColor = QColor( 50, 50, 70 );

    QFont f;
    f.setFamily("Ubuntu Mono");
    f.setWeight( 65 );
    f.setPixelSize(5);
    f.setLetterSpacing( QFont::PercentageSpacing, 120 );
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
        if( m_initialized ) return;
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

    //if( !root.hasChildNodes() ) { m_error = 1; return; }

    if(( root.tagName() == "package" )
    || ( root.tagName() == "packageB" ))
    {
        if( m_pkgeFile.endsWith( "_LS.package" )) m_isLS = true;
        else                                      m_isLS = false;

        if( m_isLS ) m_color = m_lsColor;
        else         m_color = m_icColor;

        m_width   = root.attribute( "width" ).toInt();
        m_height  = root.attribute( "height" ).toInt();
        m_area = QRect( 0, 0, 8*m_width, 8*m_height );

        m_numpins = root.attribute( "pins" ).toInt();

        for( Pin* pin : m_pin )
        {
            deletePin( pin );
            if( m_pins.contains( pin ) ) m_pins.removeAll( pin );
        }
        for( Pin* pin : m_pins ) deletePin( pin );
        m_pins.clear();
        m_ePin.clear();
        m_pin.clear();
        m_ePin.resize( m_numpins );
        m_pin.resize( m_numpins );

        if( !m_initialized )
        {
            if( root.hasAttribute("background")) setBackground( root.attribute( "background") );
            if( root.hasAttribute("type") )
            {
                /// setSubcTypeStr( root.attribute("type") );

                if( (m_subcType == Board) || (m_subcType == Shield) )
                    setTransformOriginPoint( togrid( boundingRect().center()) );
            }
            if( root.hasAttribute("name"))
            {
                QString name = root.attribute("name");
                if( name.toLower() != "package" ) m_name = name;
            }
        }
        if     ( root.tagName() == "package" )  initPackage_old( root );
        else if( root.tagName() == "packageB" ) initPackage( root );
        setName( m_name );
    }
    else{
        qDebug() <<"Chip::initChip"<<QObject::tr("Error:\nNot valid Package file:\n")<< m_pkgeFile;
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

void Chip::initPackage_old( QDomElement root )
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
            QString side  = element.attribute( "side" );
            int     pos   = element.attribute( "pos" ).toInt();

            int xpos = 0;
            int ypos = 0;
            int angle = 0;

            if( side=="left" ){
                xpos = -8;
                ypos = 8*pos;
                angle = 180;
            }
            else if( side=="top"){
                xpos = 8*pos;
                ypos = -8;
                angle = 90;
            }
            else if( side=="right" ){
                xpos =  m_width*8+8;
                ypos = 8*pos;
                angle = 0;
            }
            else if( side=="bottom" ){
                xpos = 8*pos;
                ypos =  m_height*8+8;
                angle = 270;
            }
            chipPos++;              
            addPin( id, type, label, chipPos, xpos, ypos, angle, 8 );
        }
        node = node.nextSibling();
}   }

void Chip::initPackage( QDomElement root )
{
    //setSubcTypeStr( root.attribute("type") );

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
            addPin( id, type, label, chipPos, xpos, ypos, angle, length );
        }
        node = node.nextSibling();
}   }

void Chip::addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle, int length )
{
    Pin* pin = new Pin( angle, QPoint(xpos, ypos), m_id+"-"+id, pos-1, this ); // pos in package starts at 1

    pin->setLabelText( label );

    if     ( type == "inverted" || type == "inv" ) pin->setInverted( true );
    else if( type == "unused" || type == "nc" )   pin->setUnused( true );
    else if( type == "null" )
    {
        pin->setVisible( false );
        pin->setLabelText( "" );
    }
    pin->setLength( length );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

    if( m_isLS ) pin->setLabelColor( QColor( 0, 0, 0 ) );

    m_ePin[pos-1] = pin;
    m_pin[pos-1]  = pin;
    m_pins.append( pin );
}

void Chip::deletePin( Pin* pin )
{
    if( !pin ) return;
    pin->removeConnector();
    if( pin->scene() ) Circuit::self()->removeItem( pin );
    pin->reset();
    delete pin;
}

void Chip::setLogicSymbol( bool ls )
{
    if( m_initialized && (m_isLS == ls) ) return;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();
    Circuit::self()->saveState();
    
    if(  ls && m_pkgeFile.endsWith(".package"))    m_pkgeFile.replace( ".package", "_LS.package" );
    if( !ls && m_pkgeFile.endsWith("_LS.package")) m_pkgeFile.replace( "_LS.package", ".package" );

    m_error = 0;
    Chip::initChip();
    
    if( m_error == 0 ) Circuit::self()->update();
    else               Circuit::self()->unSaveState();
}

void Chip::setSubcTypeStr( QString s )
{
    bool ok = false;
    int index = s.toInt(&ok); // OLd TODELETE
    if( !ok ) index = m_subcTypes.indexOf( s.remove("subc") );
    if( index < 0 ) index = 0;
    m_subcType = (subcType_t)index;
}

void Chip::setBackground( QString bck )
{
    m_background = bck;
    if( bck == "" ){
        if( m_BackPixmap ){
            delete m_BackPixmap;
            m_BackPixmap = NULL;
    }   }
    else{
        m_BackPixmap = new QPixmap();

        QString pixmapPath = MainWindow::self()->getFilePath("data/images");
        pixmapPath += "/"+bck;
        m_BackPixmap->load( pixmapPath );
    }
    update();
}

void Chip::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    if( m_BackPixmap )
        p->drawPixmap( m_area.x(), m_area.y(), *m_BackPixmap );
    else if( m_background != "" )
        p->drawPixmap( m_area.x(), m_area.y(), QPixmap( m_background ));
    else{
        p->drawRoundedRect( m_area, 1, 1);
        if( !m_isLS )
        {
            p->setPen( QColor( 170, 170, 150 ) );
            if( m_width == m_height )
                p->drawEllipse( 4, 4, 4, 4);
            else
                p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
}   }   }
