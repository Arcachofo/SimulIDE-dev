/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "iocomponent.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

#include "doubleprop.h"
#include "boolprop.h"

IoComponent::IoComponent( QObject* parent, QString type, QString id)
           : Component( parent, type, id )
{
    m_inHighV = 2.5;
    m_inLowV  = 2.5;
    m_ouHighV = 5;
    m_ouLowV  = 0;
    m_outValue = 0;

    m_inImp = 1e9;
    m_ouImp = 40;

    //m_rndPD = false;
    m_invInputs = false;
    m_invOutputs  = false;

    m_propSize = 1;
    m_propDelay = 10*1000; // 10 ns
    m_timeLH = 3000;
    m_timeHL = 4000;
}
IoComponent::~IoComponent(){}

QList<ComProperty*> IoComponent::inputProps()
{
    return {
new ComProperty( "", tr("Inputs:"),"","",0),
new DoubProp<IoComponent>( "Input_High_V", tr("Low to High Threshold"),"V", this, &IoComponent::inputHighV, &IoComponent::setInputHighV ),
new DoubProp<IoComponent>( "Input_Low_V" , tr("High to Low Threshold"),"V", this, &IoComponent::inputLowV,  &IoComponent::setInputLowV ),
new DoubProp<IoComponent>( "Input_Imped" , tr("Input Impedance")      ,"Ω", this, &IoComponent::inputImp,   &IoComponent::setInputImp ) };
}

QList<ComProperty*> IoComponent::outputProps()
{
    return {
new ComProperty( "", tr("Outputs:"),"","",0),
new DoubProp<IoComponent>( "Out_High_V", tr("Output High Voltage"),"V", this, &IoComponent::outHighV, &IoComponent::setOutHighV ),
new DoubProp<IoComponent>( "Out_Low_V" , tr("Output Low Voltage") ,"V", this, &IoComponent::outLowV,  &IoComponent::setOutLowV ),
new DoubProp<IoComponent>( "Out_Imped" , tr("Output Impedance")   ,"Ω", this, &IoComponent::outImp,   &IoComponent::setOutImp ) };
}

QList<ComProperty*> IoComponent::outputType()
{
    return {
new BoolProp<IoComponent>( "Inverted"      , tr("Invert Outputs"),"", this, &IoComponent::invertOuts, &IoComponent::setInvertOuts, propNoCopy ),
new BoolProp<IoComponent>( "Open_Collector", tr("Open Drain")    ,"", this, &IoComponent::openCol,    &IoComponent::setOpenCol   , propNoCopy )};
}

QList<ComProperty*> IoComponent::edgeProps()
{
    return {
new DoubProp<IoComponent>( "pd_n"  , tr("Propagation delay"),"_Gates", this, &IoComponent::propSize, &IoComponent::setPropSize ),
new DoubProp<IoComponent>( "Tpd_ps", tr("Gate Delay"),"ps", this, &IoComponent::propDelay, &IoComponent::setPropDelay ),
new DoubProp<IoComponent>( "Tr_ps" , tr("Rise Time") ,"ps", this, &IoComponent::riseTime,  &IoComponent::setRiseTime ),
new DoubProp<IoComponent>( "Tf_ps" , tr("Fall Time") ,"ps", this, &IoComponent::fallTime,  &IoComponent::setFallTime ) };
}

void IoComponent::initState()
{
    for( uint i=0; i<m_outPin.size(); ++i )
    {
        m_outPin[i]->setStateZ( false );
        m_outPin[i]->setOutState( false );
    }
    m_nextOutVal = m_outValue = 0;
    while( !m_outQueue.empty()  ) m_outQueue.pop();
    while( !m_timeQueue.empty() ) m_timeQueue.pop();
}

void IoComponent::runOutputs()
{
    if( m_outQueue.empty() )
    {
        qDebug() << "Error: IoComponent::runOutputs out Queue empty"<<m_id;
        return;
    }
    m_outValue = m_outQueue.front();
    m_outQueue.pop();

    if( !m_timeQueue.empty() )
    {
        uint64_t nextTime = m_timeQueue.front()-Simulator::self()->circTime();
        m_timeQueue.pop();
        Simulator::self()->addEvent( nextTime, m_eElement );
    }

    for( uint i=0; i<m_outPin.size(); ++i )
    {
        bool state = m_outValue & (1<<i);
        m_outPin[i]->sheduleState( state, 0 );
    }
}

void IoComponent::sheduleOutPuts( eElement* el )
{
    uint64_t delay = m_propDelay*m_propSize;
    //if( m_rndPD ) delay += (std::rand()%10)*10-45;

    if( m_outQueue.empty() )
    {
        if( m_nextOutVal == m_outValue ) return;
        Simulator::self()->addEvent( delay, el );
    }
    else          // New Event while previous Event not dispatched
    {
        if( m_nextOutVal == m_outQueue.back() ) return;

        uint64_t nextTime = Simulator::self()->circTime()+delay;
        m_timeQueue.push( nextTime );

        m_eElement = el;
    }
    m_outQueue.push( m_nextOutVal );
}

void IoComponent::setInputHighV( double volt )
{
    if( m_inHighV == volt ) return;
    m_inHighV = volt;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setInputHighV( volt );
    for( IoPin* pin : m_outPin )   pin->setInputHighV( volt );
    for( IoPin* pin : m_otherPin ) pin->setInputHighV( volt );
    Simulator::self()->resumeSim();
}

void IoComponent::setInputLowV( double volt )
{
    if( m_inLowV == volt ) return;
    m_inLowV = volt;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setInputLowV( volt );
    for( IoPin* pin : m_outPin )   pin->setInputLowV( volt );
    for( IoPin* pin : m_otherPin ) pin->setInputLowV( volt );
    Simulator::self()->resumeSim();
}

void IoComponent::setOutHighV( double volt )
{
    if( m_ouHighV == volt ) return;
    m_ouHighV = volt;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setOutHighV( volt );
    for( IoPin* pin : m_outPin )   pin->setOutHighV( volt );
    for( IoPin* pin : m_otherPin ) pin->setOutHighV( volt );
    Simulator::self()->resumeSim();
}

void IoComponent::setOutLowV( double volt )
{
    if( m_ouLowV == volt ) return;
    m_ouLowV = volt;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setOutLowV( volt );
    for( IoPin* pin : m_outPin )   pin->setOutLowV( volt );
    for( IoPin* pin : m_otherPin ) pin->setOutLowV( volt );
    Simulator::self()->resumeSim();
}

void IoComponent::setInputImp( double imp )
{
    if( m_inImp == imp ) return;
    m_inImp = imp;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setInputImp( imp );
    for( IoPin* pin : m_outPin )   pin->setInputImp( imp );
    for( IoPin* pin : m_otherPin ) pin->setInputImp( imp );
    Simulator::self()->resumeSim();
}

void IoComponent::setOutImp( double imp )
{
    if( m_ouImp == imp ) return;
    m_ouImp = imp;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin )    pin->setOutputImp( imp );
    for( IoPin* pin : m_outPin )   pin->setOutputImp( imp );
    for( IoPin* pin : m_otherPin ) pin->setOutputImp( imp );
    Simulator::self()->resumeSim();
}

void IoComponent::setInvertOuts( bool inverted )
{
    if( m_invOutputs == inverted ) return;
    m_invOutputs = inverted;
    Simulator::self()->pauseSim();
    for( uint i=0; i<m_outPin.size(); ++i ) m_outPin[i]->setInverted( inverted );
    Circuit::self()->update();
    Simulator::self()->resumeSim();
}

void IoComponent::setInvertInps( bool invert )
{
    if( m_invInputs == invert ) return;
    m_invInputs = invert;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_inPin ) pin->setInverted( invert );
    Circuit::self()->update();
    Simulator::self()->resumeSim();
}

void IoComponent::setOpenCol( bool op )
{
    if( m_openCol == op ) return;
    m_openCol = op;
    Simulator::self()->pauseSim();
    for( IoPin* pin : m_outPin )
    {
        if( op ) pin->setPinMode( openCo );
        else     pin->setPinMode( output );
    }
    Circuit::self()->update();
    Simulator::self()->resumeSim();
}

void IoComponent::setRiseTime( double time )
{
    m_timeLH = time*1e12;
    for( IoPin* pin : m_outPin ) pin->setRiseTime( m_timeLH*1.25 );
}

void IoComponent::setFallTime( double time )
{
    m_timeHL = time*1e12;
    for( IoPin* pin : m_outPin ) pin->setFallTime( m_timeHL*1.25 );
}

void IoComponent::init( QStringList pins ) // Example: pin = "IL02Name" => input, left, number 2, label = "Name"
{
    m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, m_height*8 );

    QStringList inputs;
    QStringList outputs;

    for( QString pin : pins )
    {
             if( pin.startsWith( "I" ) ) inputs.append( pin );
        else if( pin.startsWith( "O" ) ) outputs.append( pin );
        else qDebug() << " LogicComponent::init: pin name error "<<pin;
    }
    int i = m_inPin.size();
    m_inPin.resize( i+inputs.length() );
    for( QString inp : inputs ) // Example input = "L02Name"
    {
        m_inPin[i] = createPin( inp, m_id+"-in"+QString::number(i) );
        i++;
    }
    i = m_outPin.size();
    m_outPin.resize( i+outputs.length() );
    for( QString out : outputs ) // Example output = "L02Name"
    {
        m_outPin[i] = createPin( out, m_id+"-out"+QString::number(i) );
        i++;
}   }

IoPin* IoComponent::createPin( QString data, QString id ) // Example data = "L02" => left side, number 2
{
    pinMode_t mode = (data.left(1) == "I") ? input : output ;
    data.remove( 0, 1 );
    QString pos = data.left(1);
    data.remove( 0, 1 );
    int num = data.left(2).toInt();
    data.remove( 0, 2 );
    QString label = data;

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
        if( label.length() == 1
         || (label.length() == 2
          && label.startsWith("!")) )
            label.prepend(" ");
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

void IoComponent::setNumInps( uint pins, QString label, int bit0, bool number )
{ setNumPins( &m_inPin, pins, label, bit0, false, number ); }

void IoComponent::setNumOuts( uint pins, QString label, int bit0, bool number )
{ setNumPins( &m_outPin, pins, label, bit0, true, number ); }

void IoComponent::setNumPins( std::vector<IoPin*>* pinList, uint pins
                              , QString label, int bit0, bool out, bool number )
{
    uint oldSize = pinList->size();
    //if( pins == oldSize ) return;
    if( Simulator::self()->isRunning() ) CircuitWidget::self()->powerCircOff();

    int halfW = (m_width/2)*8;//m_width*8/2;
    int x           = out ? halfW+8 : -(halfW)-8;
    int angle       = out ?  0  : 180;
    QString id      = out ? "-out" : "-in";
    pinMode_t mode  = out ? output : input;

    id = m_id+id;

    if( pins < oldSize ) deletePins( pinList, oldSize-pins );
    else                 pinList->resize( pins );

    if( m_outPin.size() > m_inPin.size() ) m_height = m_outPin.size();
    else                                   m_height = m_inPin.size();

    int halfH;
    if( label.isEmpty() ) halfH = m_height*8/2; // Gates
    else
    {
        m_height += 1;
        halfH = (m_height/2)*8;
    }
    m_area = QRect(-halfW,-halfH, m_width*8, m_height*8 );

    int start = 8;
    if( label.isEmpty() ) start = 4;  // Gates
    else if( start%8 ) start +=4;

    for( uint i=0; i<pins; ++i )
    {
        int y = m_area.y() + i*8 + start;

        QString num = "";
        QString pinId = id;
        if( i < oldSize ) pinList->at(i)->setY( y );
        else{
            if( number )
            {
                pinId += QString::number(i);
                num = QString::number(i+bit0);
            }
            pinList->at(i) = new IoPin( angle, QPoint( x, y ), pinId, i, this, mode );
            initPin( pinList->at(i) );
            if( mode == output && m_invOutputs ) pinList->at(i)->setInverted( true );

            if( !label.isEmpty() ) pinList->at(i)->setLabelText( label+num );
            pinList->at(i)->setLabelColor( QColor( 0, 0, 0 ) );
    }   }
    Circuit::self()->update();
}

void IoComponent::deletePins( std::vector<IoPin*>* pinList, uint pins )
{
    uint oldSize = pinList->size();
    if( pins > oldSize ) pins = oldSize;

    uint newSize = oldSize-pins;
    for( uint i=oldSize-1; i>newSize-1; --i )
    {
        Pin* pin = pinList->at(i);
        pin->removeConnector();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        m_signalPin.removeAll( pin );
        delete pin;
    }
    pinList->resize( newSize );
}

std::vector<Pin*> IoComponent::getPins()
{
    std::vector<Pin*> pins;
    pins.reserve( m_inPin.size()+m_outPin.size()+m_otherPin.size() );
    for( Pin* pin : m_inPin    ) pins.emplace_back( pin );
    for( Pin* pin : m_outPin   ) pins.emplace_back( pin );
    for( Pin* pin : m_otherPin ) pins.emplace_back( pin );
    return pins;
}

void IoComponent::remove()
{
    for( IoPin* pin : m_inPin )    pin->removeConnector();
    for( IoPin* pin : m_outPin )   pin->removeConnector();
    for( IoPin* pin : m_otherPin ) pin->removeConnector();
    Component::remove();
}

void IoComponent::setHidden( bool hid, bool hidArea, bool hidLabel )
{
    if( m_graphical ){
        for( IoPin* pin : m_inPin )    pin->setVisible( !hid );
        for( IoPin* pin : m_outPin )   pin->setVisible( !hid );
        for( IoPin* pin : m_otherPin ) pin->setVisible( !hid );
    }
    Component::setHidden( hid, hidArea, hidLabel );
}

void IoComponent::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->drawRect( m_area );
}
