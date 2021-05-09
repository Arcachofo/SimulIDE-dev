/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "wavegen.h"
#include "pin.h"
#include "simulator.h"
#include "e-source.h"

static const char* WaveGen_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Volt Base"),
    QT_TRANSLATE_NOOP("App::Property","Duty Square"),
    QT_TRANSLATE_NOOP("App::Property","Quality"),
    QT_TRANSLATE_NOOP("App::Property","Wave Type")
};

Component* WaveGen::construct( QObject* parent, QString type, QString id )
{
    return new WaveGen( parent, type, id );
}

LibraryItem* WaveGen::libraryItem()
{
    return new LibraryItem(
        tr( "Wave Gen." ),
        tr( "Sources" ),
        "wavegen.png",
        "WaveGen",
        WaveGen::construct );
}

WaveGen::WaveGen( QObject* parent, QString type, QString id )
       : ClockBase( parent, type, id )
{
    Q_UNUSED( WaveGen_properties );
    
    m_voltBase = 0;
    m_lastVout = 0;
    m_type = Sine;
    
    setSteps( 100 );
    setDuty( 50 );
    
    connect( this, SIGNAL( freqChanged() )
           , this, SLOT( updateValues() ), Qt::UniqueConnection );
}
WaveGen::~WaveGen(){}

QList<propGroup_t> WaveGen::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Wave_Type", tr("Wave Type"),"enum"} );
    mainGroup.propList.append( {"Freq", tr("Frequency"),"Hz"} );
    mainGroup.propList.append( {"Steps", tr("Quality"),"Steps"} );
    mainGroup.propList.append( {"Duty", tr("Duty"),"\%"} );
    mainGroup.propList.append( {"Always_On", tr("Always On"),""} );

    propGroup_t elecGroup { tr("Electric") };
    elecGroup.propList.append( {"Voltage", tr("Amplitude"),"main"} );
    elecGroup.propList.append( {"Volt_Base", tr("Base Voltage"),"V"} );

    propGroup_t timeGroup { tr("Edges") };
    timeGroup.propList.append( {"Tr_ps", tr("Rise Time"),"ps"} );
    timeGroup.propList.append( {"Tf_ps", tr("Fall Time"),"ps"} );

    return {mainGroup, elecGroup, timeGroup};
}

void WaveGen::runEvent()
{
    m_time = fmod( Simulator::self()->circTime(), m_fstepsPC );
    
    if     ( m_type == Sine )     genSine();
    else if( m_type == Saw )      genSaw();
    else if( m_type == Triangle ) genTriangle();
    else if( m_type == Square )   genSquare();
    else if( m_type == Random )   genRandom();

    if( m_vOut != m_lastVout )
    {
        m_lastVout = m_vOut;

        m_output[0]->setVoltHigh( m_outHighV*m_vOut+m_voltBase );
        m_output[0]->stampOutput();
    }

    m_remainder += m_fstepsPC-(double)m_stepsPC;
    uint64_t remainerInt = m_remainder;
    m_remainder -= remainerInt;

    if( m_isRunning )
        Simulator::self()->addEvent( m_nextStep+remainerInt, this );
}

void WaveGen::genSine()
{
    m_time = qDegreesToRadians( (double)m_time*360/m_fstepsPC );
    m_vOut = sin( m_time )/2+0.5;

    m_nextStep = m_qSteps;
}

void WaveGen::genSaw()
{
    m_vOut = m_time/m_fstepsPC;

    m_nextStep = m_qSteps;
}

void WaveGen::genTriangle()
{
    if( m_time >= m_halfW )
    {
        m_vOut = 1-(m_time-m_halfW)/(m_fstepsPC-m_halfW);
    }
    else m_vOut = m_time/m_halfW;

    m_nextStep = m_qSteps;
}

void WaveGen::genSquare()
{
    if( m_vOut == 1 )
    {
        m_vOut = 0;
        m_nextStep = m_fstepsPC-m_halfW;
    }
    else
    {
        m_vOut = 1;
        m_nextStep = m_halfW;
    }
    //eNode* enode =  m_pin[0]->getEnode();
    //if( enode ) enode->saveData();
}

void WaveGen::genRandom()
{
    m_vOut = (double)rand()/(double)RAND_MAX;

    m_nextStep = m_halfW;
}

void WaveGen::updateStep()
{
    /*if(( !m_outValue )&&( m_isRunning ))
    {
        m_out->setOut( true );
    }*/
    ClockBase::updateStep();
}

void WaveGen::updateValues()
{
    setDuty( m_duty );
    setSteps( m_steps );
}

double WaveGen::duty()
{
    return m_duty;
}

void WaveGen::setDuty( double duty )
{
    if( duty > 100 ) duty = 100;
    m_duty = duty;
    
    m_halfW = m_fstepsPC*m_duty/100;
}

int WaveGen::steps()
{
    return m_steps;
}

void WaveGen::setSteps(int steps )
{
    if( steps < 10 ) steps = 10;
    m_steps = steps;

    m_qSteps  = m_stepsPC/steps;
    //qDebug()<<"WaveGen::setQuality"<<m_stepsPC<<q <<m_qSteps;
}

void WaveGen::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    if (  m_isRunning )
        p->setBrush( QColor( 250, 200, 50 ) );
    else
        p->setBrush( QColor( 230, 230, 255 ) );

    p->drawRoundedRect( m_area,2 ,2 );

    QPen pen;
    pen.setWidth(1);
    p->setPen( pen );
    
    p->drawLine(-11, 3,-5, -3 );
    p->drawLine( -5,-3,-5,  3 );
    p->drawLine( -5, 3, 1, -3 );
    p->drawLine(  1,-3, 1,  3 );
    p->drawLine(  1, 3, 4,  0 );
}

#include "moc_wavegen.cpp"

