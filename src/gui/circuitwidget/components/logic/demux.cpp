/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "demux.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "connector.h"
#include "pin.h"

Component* Demux::construct( QObject* parent, QString type, QString id )
{
        return new Demux( parent, type, id );
}

LibraryItem* Demux::libraryItem()
{
    return new LibraryItem(
        tr( "Demux" ),
        tr( "Logic/Converters" ),
        "demux.png",
        "Demux",
        Demux::construct );
}

Demux::Demux( QObject* parent, QString type, QString id )
     : LogicComponent( parent, type, id )
     , eDemux( id )
{
    m_width  = 4;
    m_height = 10;

    m_addrBits = 3;

    QStringList pinList;

    pinList // Inputs:
            << "ID03S0"
            << "ID02 S1"
            << "ID01  S2"
            
            << "IL05 DI"
            
            << "IU01OE "
            
            // Outputs:
            << "OR01O0 "
            << "OR02O1 "
            << "OR03O2 "
            << "OR04O3 "
            << "OR05O4 "
            << "OR06O5 "
            << "OR07O6 "
            << "OR08O7 "
            ;
    init( pinList );
    m_area = QRect( -(m_width/2)*8-1, -(m_height/2)*8-8-1, m_width*8+2, m_height*8+16+2 );
    
    eLogicDevice::createOutEnablePin( m_inPin[4] );    // IOutput Enable
    
    for( int i=0; i<4; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    for( int i=0; i<8; i++ )
        eLogicDevice::createOutput( m_outPin[i] );
}
Demux::~Demux(){}

QList<propGroup_t> Demux::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Address_Bits", tr("Address Size"),"Bits"} );
    mainGroup.propList.append( {"Inverted", tr("Invert Outputs"),""} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    pg.prepend( mainGroup );
    return pg;
}

void Demux::setAddrBits( int bits )
{
    if( m_addrBits == bits ) return;

    if     ( bits < 1 ) bits = 1;
    else if( bits > 3 ) bits = 3;

    int channels = pow( 2, bits );
    m_addrBits = bits;

    m_height = channels+2;

    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    for( int i=0; i<3; ++i )
    {
        Pin* pin = m_inPin[i];
        if( i < bits )
        {
            pin->setVisible( true );
            pin->setY( m_height*8/2+8 );
            if( i == 0 )
            {
                if( bits == 1 )
                {
                    pin->setX( 0 );
                    pin->setLabelText(" S0");
                }else{
                    pin->setX( 8 );
                    pin->setLabelText("S0");
                }
            }
            pin->isMoved();
            pin->setLabelPos();
        }
        else{
            if( pin->connector() ) pin->connector()->remove();
            pin->setVisible( false );
        }
    }
    for( int i=0; i<8; ++i )
    {
        Pin* pin = m_outPin[i];
        if( i < channels )
        {
            pin->setVisible( true );
            pin->setY( i*8-(bits+bits/3)*8 );
            pin->isMoved();
            pin->setLabelPos();
        }
        else{
            if( pin->connector() ) pin->connector()->remove();
            pin->setVisible( false );
        }
    }
    m_inPin[4]->setY( -m_height*8/2-8 ); // OE
    m_inPin[4]->isMoved();
    m_inPin[4]->setLabelPos();

    m_area = QRect( -(m_width/2)*8-1, -(m_height/2)*8-8-1, m_width*8+2, m_height*8+16+2 );
    Circuit::self()->update();
}

QPainterPath Demux::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-(m_width/2)*8,-(m_height/2)*8+2 )
           << QPointF(-(m_width/2)*8, (m_height/2)*8-2 )
           << QPointF( (m_width/2)*8, (m_height/2)*8+6 )
           << QPointF( (m_width/2)*8,-(m_height/2)*8-6 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Demux::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    int w = m_width*8/2;
    int h = m_height*8/2;

    QPointF points[4] = {
        QPointF(-w,-h+2 ),
        QPointF(-w, h-2 ),
        QPointF( w, h+6 ),
        QPointF( w,-h-6 )};

    p->drawPolygon(points, 4);
}

#include "moc_demux.cpp"
