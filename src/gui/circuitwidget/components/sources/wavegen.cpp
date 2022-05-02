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

#include <QPainter>
#include <QtMath>
//#include <math.h>

#include "wavegen.h"
#include "iopin.h"
#include "simulator.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

QStringList WaveGen::m_waves = {tr("Sine"),tr("Saw"),tr("Triangle"),tr("Square"),tr("Random")};

Component* WaveGen::construct( QObject* parent, QString type, QString id )
{ return new WaveGen( parent, type, id ); }

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
    m_voltBase = 0;
    m_lastVout = 0;
    m_waveType = Sine;
    
    setSteps( 100 );
    setDuty( 50 );

    remPropGroup( tr("Main") );
    addPropGroup( { tr("Main"), {
new IntProp <WaveGen>( "Wave_Type", tr("Wave Type"),""      , this, &WaveGen::waveType, &WaveGen::setWaveType, "enum" ),
new DoubProp<WaveGen>( "Freq"     , tr("Frequency"),"Hz"    , this, &WaveGen::freq,     &WaveGen::setFreq ),
new IntProp <WaveGen>( "Steps"    , tr("Quality")  ,"_Steps", this, &WaveGen::steps,    &WaveGen::setSteps ),
new DoubProp<WaveGen>( "Duty"     , tr("Duty")     ,"_\%"   , this, &WaveGen::duty,     &WaveGen::setDuty ),
new BoolProp<WaveGen>( "Always_On", tr("Always On"),""      , this, &WaveGen::alwaysOn, &WaveGen::setAlwaysOn )
    }} );
    addPropGroup( { tr("Electric"), {
new DoubProp<WaveGen>( "Semi_Ampli", tr("Semi Amplitude"),"V", this, &WaveGen::semiAmpli, &WaveGen::setSemiAmpli ),
new DoubProp<WaveGen>( "Mid_Volt"  , tr("Middle Voltage"),"V", this, &WaveGen::midVolt,   &WaveGen::setMidVolt )
    }} );
}
WaveGen::~WaveGen(){}

bool WaveGen::setPropStr( QString prop, QString val )
{
    if( prop =="Volt_Base" ) m_voltBase = val.toDouble(); //  Old: TODELETE
    else return ClockBase::setPropStr( prop, val );
    return true;
}

void WaveGen::runEvent()
{
    m_time = fmod( (Simulator::self()->circTime()-m_lastTime), m_fstepsPC );
    
    if     ( m_waveType == Sine )     genSine();
    else if( m_waveType == Saw )      genSaw();
    else if( m_waveType == Triangle ) genTriangle();
    else if( m_waveType == Square )   genSquare();
    else if( m_waveType == Random )   genRandom();

    if( m_vOut != m_lastVout )
    {
        m_lastVout = m_vOut;

        if( m_waveType == Square )
        {
            m_outpin->setOutHighV( m_voltage );
            m_outpin->setOutLowV( m_voltBase );
            m_outpin->sheduleState( m_vOut );
        }else{
            m_outpin->setOutHighV( m_voltage*m_vOut+m_voltBase );
            m_outpin->setOutState( true );
        }
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
    if( m_time >= m_halfW ) m_vOut = 1-(m_time-m_halfW)/(m_fstepsPC-m_halfW);
    else                    m_vOut = m_time/m_halfW;
    m_nextStep = m_qSteps;
}

void WaveGen::genSquare()
{
    if( m_vOut == 1 )
    {
        m_vOut = 0;
        m_nextStep = m_fstepsPC-m_halfW;
    }else{
        m_vOut = 1;
        m_nextStep = m_halfW;
}   }

void WaveGen::genRandom()
{
    m_vOut = (double)rand()/(double)RAND_MAX;
    m_nextStep = m_halfW;
}

void WaveGen::setDuty( double duty )
{
    if( duty > 100 ) duty = 100;
    m_duty = duty;
    m_halfW = m_fstepsPC*m_duty/100;
}

void WaveGen::setSteps( int steps )
{
    if( steps < 10 ) steps = 10;
    m_steps = steps;
    m_qSteps  = m_stepsPC/steps;
}

void WaveGen::setFreq( double freq )
{
    ClockBase::setFreq( freq );
    setDuty( m_duty );
    setSteps( m_steps );
}

void WaveGen::setWaveType( int type )
{
    if( type < 0 || type > m_waves.size() ) type = 0;
    m_waveType = (wave_type)type;
    //updateProperty(); // Update label
}

QStringList WaveGen::getEnums( QString e )
{
    if( e == "Wave_Type" ) return m_waves;
    else return CompBase::getEnums( e );
}

void WaveGen::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;
    Component::paint( p, option, widget );

    if (  m_isRunning ) p->setBrush( QColor( 250, 200, 50 ) );
    else                p->setBrush( QColor( 230, 230, 255 ) );

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

