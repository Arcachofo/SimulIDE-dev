/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#include "iocomponent.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"

IoComponent::IoComponent( QObject* parent, QString type, QString id)
           : Component( parent, type, id )
{
    m_inHighV = 2.5;
    m_inLowV  = 2.5;
    m_ouHighV   = 5;
    m_ouLowV    = 0;
    m_outValue   = 0;

    m_inImp = 1e9;
    m_ouImp   = 40;

    m_rndPD = false;
    m_invInputs = false;
    m_invOutputs  = false;

    m_propDelay = 10*1000; // 10 ns
    m_timeLH = 3000;
    m_timeHL = 4000;
}
IoComponent::~IoComponent(){}

QList<propGroup_t> IoComponent::propGroups()
{
    propGroup_t elecGroup { tr("Electric") };
    elecGroup.propList.append( {"", tr("Inputs:"),""} );
    elecGroup.propList.append( {"Input_High_V", tr("Low to High Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Low_V", tr("High to Low Threshold"),"V"} );
    elecGroup.propList.append( {"Input_Imped", tr("Input Impedance"),"Ω"} );
    elecGroup.propList.append( {"", tr("Outputs:"),""} );
    elecGroup.propList.append( {"Out_High_V", tr("Output High Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Low_V", tr("Output Low Voltage"),"V"} );
    elecGroup.propList.append( {"Out_Imped", tr("Output Impedance"),"Ω"} );

    propGroup_t edgeGroup { tr("Edges") };
    edgeGroup.propList.append( {"Tpd_ps", tr("Propagation Delay"),"ps"} );
    edgeGroup.propList.append( {"Tr_ps", tr("Rise Time"),"ps"} );
    edgeGroup.propList.append( {"Tf_ps", tr("Fall Time"),"ps"} );

    return {elecGroup, edgeGroup};
}

void IoComponent::initState()
{
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutState( false );

    m_outStep = 0;
    m_outValue = 0;
    m_nextOutVal = 0;
}

void IoComponent::runOutputs()
{
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        bool state = m_nextOutVal & (1<<i);
        bool oldst = m_outValue   & (1<<i);

        if( state != oldst )
        {
            /*if( m_outStep == 0 )
            {
                eNode* enode =  m_output[i]->getPin()->getEnode();
                if( enode ) enode->saveData();
            }
            else */
                m_outPin[i]->setOutState( state, true );
        }
    }
    /*if( m_outStep == 0 )
    {
        m_outStep = 1;
        Simulator::self()->addEvent( m_timeLH*1.25, this );
    }
    else*/
    {
        //m_outStep = 0;
        m_outValue = m_nextOutVal;
    }
}

void IoComponent::sheduleOutPuts(  eElement* el )
{
    if( m_nextOutVal == m_outValue ) return;

    if( m_rndPD )Simulator::self()->addEvent( m_propDelay+(std::rand()%2), el );
    else         Simulator::self()->addEvent( m_propDelay, el );
}

void IoComponent::setInputHighV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_inHighV = volt;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputHighV( volt );

    ///if( m_clockPin) m_clockPin->setInputHighV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setInputLowV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_inLowV = volt;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputLowV( volt );

    ///if( m_clockPin) m_clockPin->setInputLowV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setOutHighV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_ouHighV = volt;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutHighV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setOutLowV( double volt )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_ouLowV = volt;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutLowV( volt );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setInputImp( double imp )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_inImp = imp;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInputImp( imp );

    ///if( m_clockPin) m_clockPin->setInputImp( imp );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setOutImp( double imp )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    if( m_ouImp == imp ) return;

    m_ouImp = imp;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setOutputImp( imp );

    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setInvertOuts( bool inverted )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_invOutputs = inverted;
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setInverted( inverted );

    Circuit::self()->update();
    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setInvertInps( bool invert )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_invInputs = invert;
    for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->setInverted( invert );

    Circuit::self()->update();
    if( pauseSim ) Simulator::self()->resumeSim();
}

void IoComponent::setOpenCol( bool op )
{
    m_openCol = op;

    for( uint i=0; i<m_outPin.size(); ++i )
    {
        if( op ) m_outPin[i]->setPinMode( open_col );
        else     m_outPin[i]->setPinMode( output );
    }
}

void IoComponent::init( QStringList pins )
{
    m_area = QRect(-(m_width*8/2),-(m_height*8/2), m_width*8, m_height*8 );

    QStringList inputs;                                    // Input Pins
    QStringList outputs;                                  // Output Pins

    // Example: pin = "IL02Name" => input, left, number 2, label = "Name"

    for( QString pin : pins )
    {
             if( pin.startsWith( "I" ) ) inputs.append(  pin.remove(0,1) );
        else if( pin.startsWith( "O" ) ) outputs.append( pin.remove(0,1) );
        else qDebug() << " LogicComponent::init: pin name error "<<pin;
    }
    //setNumInps( inputs.length(), " I" ); // Create Input Pins
    m_inPin.resize( inputs.length() );
    int i = 0;
    for( QString inp : inputs ) // Example input = "L02Name"
    {
        QString pinPos  = inp.left(3);      // Pin position
        QString label = inp.remove(0,3);  // Pin name
        m_inPin[i] = createPin( pinPos, m_id+"-in"+QString::number(i), label, input );
        i++;
    }
    //setNumOuts( outputs.length() ); // Create Output Pins
    m_outPin.resize( outputs.length() );
    i = 0;
    for( QString out : outputs ) // Example output = "L02Name"
    {
        QString pin   = out.left(3);      // Pin position
        QString label = out.remove(0,3);  // Pin name
        m_outPin[i] = createPin( pin, m_id+"-out"+QString::number(i), label, output );
        i++;
    }
}

IoPin* IoComponent::createPin(QString data, QString id, QString label, pinMode_t mode )
{
    // Example data = "L02" => left side, number 2

    QString pos = data.left(1);
    int     num = data.remove(0,1).toInt();

    int angle = 0;
    int x = 0;
    int y = 0;

    if( pos == "U" )        // Up
    {
        angle = 90;
        x = m_area.x() + num*8;
        y = m_area.y() - 8;
    }
    else if( pos == "L")    // Left
    {
        angle = 180;
        x = m_area.x() - 8;
        y = m_area.y() + num*8;
    }
    if( pos == "D" )        // Down
    {
        angle = 270;
        x = m_area.x() + num*8;
        y = m_area.y() + m_height*8 + 8;
    }
    else if( pos == "R")    // Right
    {
        x = m_area.x() + m_width*8+8;
        y = m_area.y() + num*8;
    }
    IoPin* pin = new IoPin( angle, QPoint( x, y ), id, 0, this, mode );
    pin->setLabelColor( QColor( 0, 0, 0 ) );
    pin->setLabelText( label );
    initPin( pin );
    return pin;
}

void IoComponent::initPin( IoPin* pin )
{
    pin->setInputHighV( m_inHighV );
    pin->setInputLowV( m_inLowV );
    pin->setInputImp( m_inImp );
    pin->setOutHighV( m_ouHighV );
    pin->setOutLowV( m_ouLowV );
    pin->setOutputImp( m_ouImp  );
}

void IoComponent::setNumInps( uint pins, QString label )
{
    setNumPins( &m_inPin, pins, label, false );
}

void IoComponent::setNumOuts( uint pins, QString label )
{
    setNumPins( &m_outPin, pins, label, true );
}

void IoComponent::setNumPins( std::vector<IoPin*>* pinList, uint pins
                              , QString label, bool out )
{
    uint oldSize = pinList->size();
    if( pins == oldSize ) return;
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    int x           = out ? m_width*8/2+8 : -(m_width*8/2)-8;
    int angle       = out ?  0  : 180;
    QString preLab  = out ? ""  : " ";
    QString PostLab = out ? " " : "";
    QString id      = out ? "-out" : "-in";
    pinMode_t mode  = out ? output : input;

    if( pins < oldSize ) deletePins( pinList, oldSize-pins );
    else                 pinList->resize( pins );

    if( m_outPin.size() > m_inPin.size() ) m_height = m_outPin.size();
    else                                   m_height = m_inPin.size();
    if( !label.isEmpty() ) m_height += 1;

    int start = (m_height-pins)*8/2+4;

    for( uint i=0; i<pins; ++i )
    {
        int y = -(int)m_height*8/2 + start+i*8;

        if( i < oldSize ) pinList->at(i)->setY( y );
        else{
            QString num = QString::number(i);
            pinList->at(i) = new IoPin( angle, QPoint( x, y), m_id+id+num, i, this, mode );
            initPin( pinList->at(i) );

            if( !label.isEmpty() ) pinList->at(i)->setLabelText( preLab+label+num+PostLab );
            pinList->at(i)->setLabelColor( QColor( 0, 0, 0 ) );
        }
    }
    m_area = QRect(-(m_width*8/2),-(m_height*8/2), m_width*8, m_height*8 );
    Circuit::self()->update();
}

void IoComponent::deletePins( std::vector<IoPin*>* pinList, uint pins )
{
    uint oldSize = pinList->size();
    if( pins > oldSize ) pins = oldSize;

    uint newSize = oldSize-pins;

    for( uint i=oldSize-1; i>newSize-1; --i )
    {
        pinList->at(i)->removeConnector();
        if( pinList->at(i)->scene() ) Circuit::self()->removeItem( pinList->at(i) );
        pinList->at(i)->reset();
        delete pinList->at(i);
    }
    pinList->resize( newSize );
}

void IoComponent::remove()
{
    for( uint i=0; i<m_inPin.size(); i++ )  m_inPin[i]->removeConnector();
    for( uint i=0; i<m_outPin.size(); i++ ) m_outPin[i]->removeConnector();

    Component::remove();
}
void IoComponent::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );
}

#include "moc_iocomponent.cpp"
