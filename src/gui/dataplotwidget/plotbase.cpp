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

#include "plotbase.h"
#include "plotdisplay.h"
#include "simulator.h"
#include "circuit.h"
#include "circuitwidget.h"


PlotBase::PlotBase( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eElement( id )
{
    m_color[0] = QColor( 240, 240, 100 );
    m_color[1] = QColor( 220, 220, 255 );
    m_color[2] = QColor( 255, 210, 90  );
    m_color[3] = QColor( 000, 245, 160 );
    m_color[4] = QColor( 255, 255, 255 );

    m_baSizeX = 135;
    m_baSizeY = 135;

    QString n;
    for( int i=1; i<9; ++i ) // Condition simple to script: ch1l to (ch1==1)
    {
        n = QString::number(i);
        m_condTo["ch"+n+"l"] = "(ch"+n+"==1)";
        m_condTo["ch"+n+"r"] = "(ch"+n+"==2)";
        m_condTo["ch"+n+"h"] = "(ch"+n+"==3)";
        m_condTo["ch"+n+"f"] = "(ch"+n+"==4)";
    }

    Simulator::self()->addToUpdateList( this );
}
PlotBase::~PlotBase()
{
    Simulator::self()->remFromUpdateList( this );

    for( int i=0; i<m_numChannels; i++ ) delete m_channel[i];
}

void PlotBase::setBaSizeX( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeX = size;
    expand( m_expand );
}

void PlotBase::setBaSizeY( int size )
{
    if( size < 135 ) size = 135;
    m_baSizeY = size;
    expand( m_expand );
}

void PlotBase::setTimeDiv( uint64_t td )
{
    m_timeDiv = td;
    m_display->setTimeDiv( td );
}

QStringList PlotBase::tunnels()
{
    QStringList list;
    for( int i=0; i<m_numChannels; ++i ) list << m_channel[i]->m_chTunnel;
    return list;
}

void PlotBase::updateConds(  QString conds )
{
    m_conditions = conds;
    conds = conds.toLower();

    QString n;
    for( int i=1; i<9; ++i ) // Condition simple to script: ch1l to (ch1==1)
    {
        n = QString::number(i);
        conds.replace( "ch"+n+"l", m_condTo.value("ch"+n+"l") );
        conds.replace( "ch"+n+"r", m_condTo.value("ch"+n+"r") );
        conds.replace( "ch"+n+"h", m_condTo.value("ch"+n+"h") );
        conds.replace( "ch"+n+"f", m_condTo.value("ch"+n+"f") );
    }

    m_condProgram = QScriptProgram( conds );
}

void PlotBase::conditonMet( int ch, cond_t cond )
{
    m_engine.globalObject().setProperty( "ch"+QString::number(ch+1), QScriptValue( (int)cond ) );

    if( Simulator::self()->simState() <= SIM_PAUSED ) return;
    if( m_condProgram.isNull() ) return;

    // Check if condition met:
    bool pause = m_engine.evaluate( m_condProgram ).toBool();
    if( pause )
    {
        m_risEdge = Simulator::self()->circTime();
        CircuitWidget::self()->pauseSim();
    }

    //if( m_trigger != 8 ) return;

    /*if( m_conditions == m_condTarget ) // All conditions met
    {                                  // Trigger Pause Simulation
        m_risEdge = Simulator::self()->circTime();
        CircuitWidget::self()->pauseSim();
    }*/
    /*else  // Rising will be High and Falling Low in next cycles
    {
        if     ( cond == C_RISING )  m_conditions[ch] = C_HIGH;
        else if( cond == C_FALLING ) m_conditions[ch] = C_LOW;
    }*/
}

void PlotBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    //p->setBrush( Qt::darkGray );
    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 4, 4 );
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( Qt::white );
    p->setPen(pen);
}

#include "moc_plotbase.cpp"
