/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include "adc.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"

static const char* ADC_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Vref")
};

Component* ADC::construct( QObject* parent, QString type, QString id )
{
    return new ADC( parent, type, id );
}

LibraryItem* ADC::libraryItem()
{
    return new LibraryItem(
        tr( "ADC" ),
        tr( "Logic/Other Logic" ),
        "1to3.png",
        "ADC",
        ADC::construct );
}

ADC::ADC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
   , eElement( id )
{
    Q_UNUSED( ADC_properties );
    
    m_width  = 4;
    m_height = 9;

    setNumOuts( 8 );    // Create Output Pins
    setNumInps( 1 );

    setMaxVolt( 5 );

    setLabelPos(-16,-80, 0);
}
ADC::~ADC(){}

QList<propGroup_t> ADC::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Num_Bits", tr("Size"),"Bits"} );
    mainGroup.propList.append( {"Vref", tr("Reference Voltage"),"V"} );

    QList<propGroup_t> pg = LogicComponent::propGroups();
    for( int i=0;i<4; ++i ) pg.first().propList.removeFirst(); //remove Inputs.
    pg.prepend( mainGroup );
    return pg;
}

void ADC::stamp()
{
    m_inPin[0]->changeCallBack( this );
    LogicComponent::stamp( this );
}

void ADC::voltChanged()
{
    double volt = m_inPin[0]->getVolt();
    m_nextOutVal = (int)(volt*m_maxValue/m_maxVolt+0.1);
    m_outStep = 0;

    if( m_outValue != m_nextOutVal )
        Simulator::self()->addEvent( m_propDelay, this );
}

void ADC::setNumOuts( int outs )
{
    if( outs < 1 ) return;
    m_maxValue = pow( 2, outs )-1;
    LogicComponent::setNumOuts( outs, "D" );
}

#include "moc_adc.cpp"
