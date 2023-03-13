/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "socket.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "e-node.h"

Component* Socket::construct( QObject* parent, QString type, QString id )
{ return new Socket( parent, type, id ); }

LibraryItem* Socket::libraryItem()
{
    return new LibraryItem(
        tr( "Socket" ),
        "Connectors",
        "socket.png",
        "Socket",
        Socket::construct);
}

Socket::Socket( QObject* parent, QString type, QString id )
      : ConnBase( parent, type, id )
{
    setZValue(-2 );

    m_pinType = Pin::pinSocket;

    m_color = QColor( 50, 50, 70 );

    Simulator::self()->addToSocketList( this );
    Simulator::self()->addToUpdateList( this );
}
Socket::~Socket()
{
    Simulator::self()->remFromSocketList( this );
}

void Socket::updateStep()
{
    updatePins( false );
}

void Socket::updatePins( bool connect )
{
    for( int i=0; i<m_size; i++ )
    {
        if( !m_sockPins[i]->connector() )
        {
            Pin* pin = m_sockPins[i]->connectPin( false );
            if( pin ){
                if( connect )
                {
                    qDebug()<<"Connecting"<< m_sockPins[i]->pinId()<<"To"<<pin->pinId();
                    m_connPins[i] = pin;
                    m_sockPins[i]->setConPin( pin );
                    pin->setConPin( m_sockPins[i] );
                }
                else if( !m_connPins[i] ) CircuitWidget::self()->powerCircOff();
            }
            else{
                if( m_connPins[i] )
                {
                    m_sockPins[i]->setConPin( NULL );
                    m_connPins[i]->setConPin( NULL );
                    m_connPins[i] = NULL;
                    CircuitWidget::self()->powerCircOff();
                }
            }
        }
    }
}

void Socket::updatePixmap()
{
    int angle = this->rotation();
    switch( angle ) {
        case 0:
        case 180:
        case -180:
            m_pinPixmap.load( ":/socket_V.png" );
            m_pinPixmap = m_pinPixmap.transformed( QTransform().rotate(angle) );
            break;
        case 90:
        case -90:
            m_pinPixmap.load( ":/socket_H.png" );
            m_pinPixmap = m_pinPixmap.transformed( QTransform().rotate(angle-180) );
            break;
        default: break;
    }
}
