/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                      *
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
/*
 *   Modified 2020 by santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include "ky023.h"
#include "pin.h"
#include "simulator.h"
#include "circuit.h"

#define WIDTH 40
#define HEIGHT 56
#define GAP 0
#define JOYSTICK_SIZE 36

#define VIN 5

Component* KY023::construct( QObject* parent, QString type, QString id )
{ return new KY023( parent, type, id ); }

LibraryItem* KY023::libraryItem()
{
    return new LibraryItem(
            tr( "KY-023" ),
            tr( "Perifericals" ),
            "ky-023.png",
            "KY023",
            KY023::construct);
}

KY023::KY023( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
{
    m_joystickW.setupWidget();
    m_joystickW.setFixedSize( JOYSTICK_SIZE, JOYSTICK_SIZE );
    onvaluechanged(500, 500);
    
    m_proxy = Circuit::self()->addWidget( &m_joystickW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2 + (WIDTH - JOYSTICK_SIZE) / 2, -HEIGHT/2+2 + 2*GAP) );
    
    m_button = new QToolButton();
    m_button->setMaximumSize( 10,10 );
    m_button->setGeometry(-10,-10,10,10);
    
    m_proxy_button = Circuit::self()->addWidget( m_button );
    m_proxy_button->setParentItem( this );
    m_proxy_button->setPos( QPoint(8, HEIGHT/2-13) );
    
    m_area = QRect( -WIDTH/2, -HEIGHT/2 + GAP, WIDTH, HEIGHT );
    setLabelPos(-WIDTH/2, -HEIGHT/2 - GAP, 0);
    
    m_pin.resize(3);
    
    QString pinid = id;
    pinid.append(QString("-vrx"));
    QPoint pinpos = QPoint(-12,36);
    m_vrxpin = new Pin( 270, pinpos, pinid, 0, this);
    m_vrxpin->setLabelText( " VRX" );
    m_pin[0] = m_vrxpin;
    
    pinid.append(QString("-eSource"));
    m_vrx = new eSource( pinid, m_vrxpin );
    m_vrx->setVoltHigh( VIN );
    m_vrx->setImp( 40 );
    
    pinid = id;
    pinid.append(QString("-vry"));
    pinpos = QPoint(-4,36);
    m_vrypin = new Pin( 270, pinpos, pinid, 0, this);
    m_vrypin->setLabelText( " VRY" );
    m_pin[1] = m_vrypin;

    pinid.append(QString("-eSource"));
    m_vry = new eSource( pinid, m_vrypin );
    m_vry->setVoltHigh( VIN );
    m_vry->setImp( 40 );
    
    pinid = id;
    pinid.append(QString("-sw"));
    pinpos = QPoint(4,36);
    m_swpin = new Pin( 270, pinpos, pinid, 0, this);
    m_swpin->setLabelText( " SW" );
    m_pin[2] = m_swpin;

    pinid.append(QString("-eSource"));
    m_sw = new eSource( pinid, m_swpin );
    m_sw->setVoltHigh( VIN );
    m_sw->setImp( 40 );
    
    Simulator::self()->addToUpdateList( this );
    
    connect( &m_joystickW, SIGNAL( valueChanged(int, int) ),
             this,         SLOT  ( onvaluechanged(int, int) ));
    
    connect( m_button, SIGNAL( pressed() ),
             this,     SLOT  ( onbuttonpressed() ));
    
    connect( m_button, SIGNAL( released() ),
             this,     SLOT  ( onbuttonreleased() ));
    
    initialize();
}
KY023::~KY023(){}

void KY023::onbuttonpressed()
{
    m_closed = true;
    m_changed = true;
    
    update();
}

void KY023::onbuttonreleased()
{
    m_closed = false;
    m_changed = true;
    
    update();
}

void KY023::onvaluechanged(int xValue, int yValue)
{
    m_vOutX = double(VIN * xValue) / 1000;
    m_vOutY = double(VIN * (1000 - yValue)) / 1000;
    
    m_changed = true;
}

void KY023::updateStep()
{
    if( m_changed )
    {
        m_vrx->setVoltHigh( m_vOutX );
        m_vrx->setOut( true );
        m_vrx->stampOutput();
        
        m_vry->setVoltHigh( m_vOutY );
        m_vry->setOut( true );
        m_vry->stampOutput();

        m_sw->setOut( !m_closed );
        m_sw->stampOutput();
        
        m_changed = false;
        Simulator::self()->addEvent( 0, 0l );
    }
}

void KY023::remove()
{
    delete m_vrx;
    delete m_vry;
    delete m_sw;
    Component::remove();
}

void KY023::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );
}

#include "moc_ky023.cpp"
