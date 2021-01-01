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

#include <cmath>

#include "dcmotor.h"
#include "simulator.h"
#include "pin.h"

static const char* DcMotor_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","RPM Nominal"),
    QT_TRANSLATE_NOOP("App::Property","Volt Nominal"),
};

Component* DcMotor::construct( QObject* parent, QString type, QString id )
{
    return new DcMotor( parent, type, id );
}

LibraryItem* DcMotor::libraryItem()
{
    return new LibraryItem(
        tr("Dc Motor"),
        tr("Motors"),
        "dcmotor.png",
        "DcMotor",
        DcMotor::construct );
}

DcMotor::DcMotor( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eResistor( id )
{
    Q_UNUSED( DcMotor_properties );

    m_graphical = true;
    
    m_area = QRectF( -33,-33, 66, 66 );
    m_color = QColor( 50, 50, 70 );

    m_ang  = 0;
    m_voltNom = 5;

    setRpm( 60 );

    m_pin.resize( 2 );
    
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-40,0);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-rPin"));
    pinPos = QPoint(40,0);
    m_pin[1] = new Pin( 0, pinPos, pinId, 1, this);
    m_ePin[1] = m_pin[1];

    m_idLabel->setPos(-12,-24);
    setLabelPos(-22,-48, 0);

    m_unit = "Ω";
    setResist( m_resist );

    Simulator::self()->addToUpdateList( this );

    setShowId( true );
}

DcMotor::~DcMotor()
{
    Simulator::self()->remFromUpdateList( this );
}

void DcMotor::initialize()
{
    m_ang = 0;
    m_speed = 0;
    m_delta = 0;
    m_lastTime = 0;
    m_updtTime = 0;
    m_LastVolt = 0;
}

void DcMotor::stamp()
{
     if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
     {
        eNode* enode = m_ePin[0]->getEnode();// Register for changes callback
        if( enode ) enode->voltChangedCallback( this );

        enode = m_ePin[1]->getEnode();// Register for changes callback
        if( enode ) enode->voltChangedCallback( this );
     }
}

void DcMotor::updateStep()
{
    updatePos();

    m_speed = m_delta/(m_updtTime/1e12);

    m_ang += m_motStPs*m_delta;
    m_ang = remainder( m_ang, (16.0*360.0) );

    m_delta = 0;
    m_updtTime = 0;
    update();
}

void DcMotor::voltChanged()
{
    updatePos();
}

void DcMotor::updatePos()
{
    uint64_t timePS = Simulator::self()->circTime();
    uint64_t duration = timePS-m_lastTime;
    m_updtTime += duration;
    m_lastTime = timePS;

    m_delta += (m_LastVolt/m_voltNom)*(duration/1e12);

    m_LastVolt = m_ePin[1]->getVolt() - m_ePin[0]->getVolt();
    //qDebug() << "DcMotor::updatePos()"<< delta << m_ang << m_motStPs;
}

int DcMotor::rpm()
{
    return m_rpm;
}

void DcMotor::setRpm( int rpm )
{
    if( rpm < 1 ) rpm = 1;
    m_rpm = rpm;
    m_motStPs = 16*360*rpm/60;

    update();
}

double DcMotor::volt()
{
    return m_voltNom;
}

void DcMotor::setVolt( double volt )
{
    m_voltNom = volt;
}

double DcMotor::resist()
{
    return m_value;
}

void DcMotor::setResist( double res )
{
    if( res < 1e-12 ) res = 1e-12;

    Component::setValue( res );       // Takes care about units multiplier
    eResistor::setResSafe( m_value*m_unitMult );
}

void DcMotor::setUnit( QString un )
{
    Component::setUnit( un );
    eResistor::setResSafe( m_value*m_unitMult );
}

void DcMotor::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->setBrush( QColor(50, 70, 100) );
    p->drawEllipse(-33,-33, 66, 66 );

    p->setPen( QColor(0, 0, 0) );
    p->setBrush( QColor(255, 255, 255) );
    p->drawEllipse(-28,-28, 56, 56 );

    // rotating pointer
    p->setPen ( QColor(0, 0, 0) );
    p->setBrush( QColor(50, 70, 100) );
    p->drawPie(-25,-25, 50, 50, m_ang-120, 240 );

    p->setBrush( QColor(50, 70, 100) );
    p->drawEllipse(-20,-20, 40, 40 );

    // Speed and Direction Indicator
    double speed = m_speed;
    double exedd = 0;
    if( m_speed > 1 )
    {
        exedd = m_speed -1;
        speed = 1;
    }
    else if( m_speed < -1 )
    {
        exedd = m_speed +1;
        speed = -1;
    }
    p->setPen ( QColor(50, 70, 100) );
    p->setBrush( QColor(100, 200, 70) );
    p->drawPie(-20,-20, 40, 40, 16*90, speed*16*180 );

    p->setBrush( QColor(200, 100, 70) );
    p->drawPie(-20,-20, 40, 40, -16*90, exedd*16*180 );
    //qDebug() << "DcMotor::paint"<< m_speed << speed << exedd;
}
#include "moc_dcmotor.cpp"
