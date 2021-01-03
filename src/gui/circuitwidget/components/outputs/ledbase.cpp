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

#include "ledbase.h"
#include "circuitwidget.h"
#include "connector.h"
#include "simulator.h"
#include "pin.h"

static const char* LedBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","MaxCurrent"),
    QT_TRANSLATE_NOOP("App::Property","Grounded")
};

LedBase::LedBase( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eLed( id )
{
    Q_UNUSED( LedBase_properties );
    
    m_graphical = true;
    m_overCurrent = false;
    m_grounded = false;
    m_scrEnode = 0l;
    m_counter = 0;
    m_bright = 0;
    
    m_color = QColor( Qt::black );
    setColor( yellow );
    
    m_valLabel->setEnabled( false );
    m_valLabel->setVisible( false );

    Simulator::self()->addToUpdateList( this );
}
LedBase::~LedBase() {}

void LedBase::initialize()
{
    eLed::initialize();
    update();
}

void LedBase::updateStep()
{
    uint32_t bright = m_bright;
    eLed::updateBright();
    
    if( m_bright > 255+75 )
    {
        m_bright = 255+75;
        m_counter++;
        
        if( m_counter > 4 )
        {
            m_counter = 0;
            m_overCurrent = !m_overCurrent;
            update();
        }
    }
    else if( m_overCurrent )
    {
        m_overCurrent = false;
        m_counter = 0;
        update();
    }
    else if( bright != m_bright ) update();
}

bool LedBase::grounded()
{
    return m_grounded;
}

void LedBase::setGrounded( bool grounded )
{
    if( grounded == m_grounded ) return;
    
    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    if( grounded )
    {
        Pin* pin1 = (static_cast<Pin*>(m_ePin[1]));
        if( m_ePin[1]->isConnected() ) pin1->connector()->remove();
        pin1->setEnabled( false );
        pin1->setVisible( false );

        if( !m_scrEnode )
        {
            m_scrEnode = new eNode( m_id+"Gnod" );
            m_scrEnode->setNodeNumber(0);
            Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );
        }
        m_ePin[1]->setEnode( m_scrEnode );
    }
    else
    {
        Pin* pin1 = (static_cast<Pin*>(m_ePin[1]));

        pin1->setEnabled( true );
        pin1->setVisible( true );

        m_ePin[1]->setEnode( 0l );
    }
    m_grounded = grounded;
}

void LedBase::remove()
{
    if( m_grounded ) m_ePin[1]->setEnode( 0l );
    //if( m_scrEnode ) delete m_scrEnode;
    Simulator::self()->remFromUpdateList( this ); 
    
    Component::remove();
}

void LedBase::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    QPen pen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    
    QColor color;

    if( m_overCurrent )                               // Max Current
    {
        //m_bright = 0;
        p->setBrush( Qt::white );
        color = QColor( Qt::white );
        pen.setColor( color );
    }
    else
    {
        int overBight = 100;
        
        if( m_bright > 25 )
        {
            m_bright += 15;                          // Set a Minimun Bright
            
            if( m_bright > 255 ) 
            {
                overBight += m_bright-255;
                m_bright = 255;
            }
        }
        
        color = QColor( m_bright, m_bright, overBight ); // Default = yellow
        
        if     ( m_ledColor == red )    color = QColor( m_bright,  m_bright/3, overBight );
        else if( m_ledColor == green )  color = QColor( overBight, m_bright,   m_bright*2/3 );
        else if( m_ledColor == blue )   color = QColor( overBight, m_bright/2, m_bright );
        else if( m_ledColor == orange ) color = QColor( m_bright,  m_bright*2/3, overBight );
        else if( m_ledColor == purple ) color = QColor( m_bright,  overBight,  m_bright*2/3 );
    }
    p->setPen(pen);
    drawBackground( p );
    
    pen.setColor( color );
    pen.setWidth(2.5);
    p->setPen(pen);
    p->setBrush( color );

    drawForeground( p );
}

#include "moc_ledbase.cpp"
