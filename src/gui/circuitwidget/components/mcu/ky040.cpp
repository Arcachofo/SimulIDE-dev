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

#include "ky040.h"
#include "pin.h"
#include "simulator.h"
#include "circuit.h"

#define WIDTH 40
#define HEIGHT 56
#define GAP 0
#define DIAL_SIZE 36

#define VIN 5

const bool CWseq[2][4] = { {true, false, false, true}, {false, false, true, true} };
const bool CCWseq[2][4] = { {true, true, false, false}, {false, true, true, false} };

Component* KY040::construct( QObject* parent, QString type, QString id )
{ return new KY040( parent, type, id ); }

LibraryItem* KY040::libraryItem()
{
    return new LibraryItem(
            tr( "KY-040" ),
            tr( "Perifericals" ),
            "ky-040.png",
            "KY040",
            KY040::construct);
}

KY040::KY040( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id )
{
    m_changed = false;
    m_closed = false;
    
    m_dialW.setupWidget();
    m_dialW.setFixedSize( DIAL_SIZE, DIAL_SIZE );
    
    m_seqIndex = -1;
    
    m_dial = m_dialW.dial;
    m_dial->setWrapping(true);
    setDetents(20);
    
    m_proxy = Circuit::self()->addWidget( &m_dialW );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2+(WIDTH-DIAL_SIZE)/2, -HEIGHT/2+(WIDTH-DIAL_SIZE)/2+GAP) );
    
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
    pinid.append(QString("-dt"));
    QPoint pinpos = QPoint(-4,36);
    m_dtpin = new Pin( 270, pinpos, pinid, 0, this);
    m_dtpin->setLabelText( " DT" );
    m_pin[0] = m_dtpin;
    
    pinid.append(QString("-eSource"));
    m_dt = new eSource( pinid, m_dtpin, output );
    m_dt->setVoltHigh( VIN );
    //m_dt->setImp( 40 );
    
    pinid = id;
    pinid.append(QString("-clk"));
    pinpos = QPoint(4,36);
    m_clkpin = new Pin( 270, pinpos, pinid, 0, this);
    m_clkpin->setLabelText( " CLK" );
    m_pin[1] = m_clkpin;

    pinid.append(QString("-eSource"));
    m_clk = new eSource( pinid, m_clkpin, output );
    m_clk->setVoltHigh( VIN );
    //m_clk->setImp( 40 );
    
    pinid = id;
    pinid.append(QString("-sw"));
    pinpos = QPoint(-12,36);
    m_swpin = new Pin( 270, pinpos, pinid, 0, this);
    m_swpin->setLabelText( " SW" );
    m_pin[2] = m_swpin;

    pinid.append(QString("-eSource"));
    m_sw = new eSource( pinid, m_swpin, output );
    m_sw->setVoltHigh( VIN );
    m_sw->setState( !m_closed );
    //m_sw->setImp( 40 );

    Simulator::self()->addToUpdateList( this );
    
    connect( m_dial, SIGNAL( valueChanged(int)),
             this,   SLOT  ( posChanged(int)) );
    
    connect( m_button, SIGNAL( pressed() ),
             this,     SLOT  ( onbuttonpressed() ));
    
    connect( m_button, SIGNAL( released() ),
             this,     SLOT  ( onbuttonreleased() ));
    
    initialize();
}
KY040::~KY040(){}

void KY040::updateStep()
{
    if( m_changed )
    {
        m_sw->setState( !m_closed, true );
        m_changed = false;
    }
}

void KY040::runEvent()
{
    if( m_seqIndex >= 0 )
    {
        bool dtOuput;
        bool clkOuput;

        if( m_clockwise )
        {
            dtOuput  = CWseq[0][m_seqIndex];
            clkOuput = CWseq[1][m_seqIndex];
        }
        else
        {
            dtOuput  = CCWseq[0][m_seqIndex];
            clkOuput = CCWseq[1][m_seqIndex];
        }
        m_dt->setState( dtOuput, true );
        m_clk->setState( clkOuput, true );

        m_seqIndex++;
        if( m_seqIndex >= 4 ) m_seqIndex = -1;
    }
    Simulator::self()->addEvent( 100*1e6, this );
}

void KY040::onbuttonpressed()
{
    m_closed = true;
    m_changed = true;
    
    update();
}

void KY040::onbuttonreleased()
{
    m_closed = false;
    m_changed = true;
    
    update();
}

int KY040::detents () {
    return m_detents;
}

void KY040::setDetents( int val ) 
{
    if( val < 10 ) val = 10;

    m_detents = val;
    m_dial->setMinimum(1);
    m_dial->setMaximum(val);
    m_dial->setValue(1);
    m_dial->setSingleStep(1);
}

void KY040::initialize()
{
    m_prevDialVal = 0;
    Simulator::self()->addEvent( 1, this );
}

void KY040::posChanged( int value )
{
    if( m_prevDialVal == value ) return;

    m_seqIndex = 0;
    
    m_clockwise = ( m_prevDialVal < value
                 || (m_prevDialVal > m_detents-3
                    && value < m_detents-3 ));
    
    m_prevDialVal = value;
}

void KY040::remove()
{
    delete m_sw;
    delete m_clk;
    delete m_dt;
    Component::remove();
}

void KY040::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush(QColor( 50, 50, 70 ));
    p->drawRoundedRect( m_area, 2, 2 );
}

#include "moc_ky040.cpp"
