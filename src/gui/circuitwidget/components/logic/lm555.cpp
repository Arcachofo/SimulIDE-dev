/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "lm555.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "iopin.h"


Component* Lm555::construct( QObject* parent, QString type, QString id )
{
        return new Lm555( parent, type, id );
}

LibraryItem* Lm555::libraryItem()
{
    return new LibraryItem(
        tr( "lm555" ),
        tr( "Logic/Other Logic" ),
        "ic2.png",
        "Lm555",
        Lm555::construct );
}

Lm555::Lm555( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    m_area = QRect( 0, 0, 8*4, 8*5 );
    m_color = QColor( 50, 50, 70 );
    
    m_pin.resize( 8 );
    
    m_pin[0] = new Pin( 180, QPoint(-8, 8*1), id+"-ePin0", 0, this );
    m_pin[0]->setLabelText( "Gnd" );
    m_pin[0] = m_pin[0];

    m_pin[1] = new Pin( 180, QPoint(-8, 8*2), id+"-ePin1", 1, this );
    m_pin[1]->setLabelText( "Trg" );
    m_pin[1] = m_pin[1];

    m_output = new IoPin( 180, QPoint(-8, 8*3), id+"-ePin2", 2, this, output );
    m_output->setLabelText( "Out" );
    m_output->setOutputImp( 10 );
    m_output->setOutState( true );
    m_pin[2] = m_output;

    m_pin[3] = new Pin( 180, QPoint(-8, 8*4), id+"-ePin3", 3, this );
    m_pin[3]->setLabelText( "Rst" );

    m_cv = new IoPin( 0, QPoint(4*8+8, 8*4), id+"-ePin4", 4, this, output );
    m_cv->setLabelText( "CV" );
    m_cv->setOutputImp( 10 );
    m_cv->setOutState( true );
    m_pin[4] = m_cv;

    m_pin[5] = new Pin( 0, QPoint(4*8+8, 8*3), id+"-ePin5", 5, this );
    m_pin[5]->setLabelText( "Thr" );

    m_dis = new IoPin( 0, QPoint(4*8+8, 8*2), id+"-ePin6", 6, this, input );
    m_pin[6]->setLabelText( "Dis" );
    m_pin[6] = m_dis;

    m_pin[7] = new Pin( 0, QPoint(4*8+8, 8*1),id+"-ePin7", 7, this );
    m_pin[7]->setLabelText( "Vcc" );

    m_propDelay = 10*1000; // 10 ns
    
}
Lm555::~Lm555()
{
}

void Lm555::stamp()
{
    for( int i=0; i<8; ++i )
    {
        if( i == 2 ) continue; // Output
        if( i == 6 ) continue; // Discharge

        if( m_pin[i]->isConnected() ) m_pin[i]->getEnode()->addToNoLinList(this);
    }
}

void Lm555::initialize()
{
    m_outState = false;

    m_voltLast = 0;
    m_voltNegLast = 0;
    m_voltHightLast = 0;
    m_disImpLast = cero_doub;
}

void Lm555::voltChanged()
{
    bool changed = false;
    double voltPos = m_pin[7]->getVolt();
    m_voltNeg = m_pin[0]->getVolt();
    m_volt    = voltPos - m_voltNeg;

    double reftTh = m_pin[4]->getVolt();
    double reftTr = reftTh/2;

    if( m_voltLast != m_volt ) changed = true;

    double voltTr = m_pin[1]->getVolt();
    double voltTh = m_pin[5]->getVolt();

    double voltRst = m_pin[3]->getVolt();

    bool reset = ( voltRst < (m_voltNeg+0.7) );
    bool th    = ( voltTh > reftTh );
    bool tr    = ( reftTr > voltTr );

    bool outState = m_outState;

    if     ( reset )     outState = false;
    else if( tr )        outState =  true;
    else if( !tr && th ) outState =  false;

    //qDebug() << "eLm555::setVChanged" << outState<<"th"<<th<<"tr"<<tr;
    if( outState != m_outState )
    {
        m_outState = outState;
        m_voltHight = m_voltNeg;

        if( outState )
        {
            m_voltHight = voltPos - 1.7;
            if( m_voltHight < m_voltNeg ) m_voltHight = m_voltNeg;
            m_disImp = high_imp;
        }
        else m_disImp = 1;

        changed = true;

        //qDebug() << "eLm555::setVChanged" << outState<<reset<<th<<tr;
    }
    if( changed ) Simulator::self()->addEvent( m_propDelay, this );
}

void Lm555::runEvent()
{
    if( m_voltLast != m_volt )
    {
        m_cv->setOutHighV( m_volt*2/3 );
        m_cv->stampOutput();
        m_voltLast = m_volt;
    }
    if( m_voltNegLast != m_voltNeg )
    {
        m_dis->setOutHighV( m_voltNeg );
        m_dis->stampOutput();
        m_voltNegLast = m_voltNeg;
    }
    if( m_voltHightLast != m_voltHight )
    {
        m_output->setOutHighV( m_voltHight );
        m_output->stampOutput();
        m_voltHightLast = m_voltHight;
    }
    if( m_disImpLast != m_disImp )
    {
        m_dis->setImp( m_disImp );
        m_disImpLast = m_disImp;
    }
}

void Lm555::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area, 1, 1);
    
    p->setPen( QColor( 170, 170, 150 ) );
    p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
}

#include "moc_lm555.cpp"
