/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "header.h"
#include "itemlibrary.h"
//#include "circuitwidget.h"
//#include "simulator.h"
//#include "circuit.h"
//#include "e-node.h"
//#include "pin.h"

#include "boolprop.h"

#define tr(str) simulideTr("Header",str)

Component* Header::construct( QString type, QString id )
{ return new Header( type, id ); }

LibraryItem* Header::libraryItem()
{
    return new LibraryItem(
        tr("Header"),
        "Connectors",
        "header.png",
        "Header",
        Header::construct);
}

Header::Header( QString type, QString id )
      : ConnBase( type, id )
{
    this->setZValue(-1 );

    m_pinType = Pin::pinHeader;
    setPointDown( false );

    m_color = QColor( 65, 65, 65 );

    this->addProperty( tr("Main"),
        new BoolProp<Header>("Down", tr("Downwards"),""
                            , this, &Header::pointDown, &Header::setPointDown ));
}
Header::~Header(){}

void Header::setPointDown( bool down )
{
    m_pointDown = down;
    Pin::pinType_t type;

    type = down ? Pin::pinNormal :Pin::pinHeader;

    for( int i=0; i<m_size; i++ )
    {
        m_sockPins[i]->setFlag( QGraphicsItem::ItemStacksBehindParent, down );
        m_sockPins[i]->setPinType( type );
    }
    updatePixmap();
}

void Header::updatePins()
{
    setPointDown( m_pointDown );
}

void Header::updatePixmap()
{
    if( m_pointDown ) m_pinPixmap.load( ":/header_D.png" );
    else              m_pinPixmap.load( ":/header_U.png" );

    int angle = this->rotation();
    switch( angle ) {
        case 0:
        case 180:
        case -180:
            m_pinPixmap = m_pinPixmap.transformed( QTransform().rotate(angle) );
            break;
        case 90:
        case -90:
            m_pinPixmap = m_pinPixmap.transformed( QTransform().rotate(angle-180) );
            break;
        default: break;
    }
}
