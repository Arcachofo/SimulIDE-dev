/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QDebug>

#include "esp32pin.h"
#include "qemumodule.h"
#include "simulator.h"

Esp32Pin::Esp32Pin( int i, QString id, QemuDevice* mcu, IoPin* dummyPin )
        : IoPin( 0, QPoint(0,0), mcu->getId()+"-"+id, i, mcu, input )
        //, QemuModule( mcu, i )
{
    //m_id     = id;

    m_pullResist = 4.5e4; // Nominal ESP weak pull resistance. 45k

    double vdd = 3.3; //m_port->getMcu()->vdd();
    m_outHighV = vdd;
    m_inpHighV = vdd/2;
    m_inpLowV  = vdd/2;

    m_pullUp = 0;
    m_pullDown = 0;

    m_pinMask = 1<<i;

    m_dummyPin =  dummyPin;

    m_pinLabel = id;

    for( int i=0; i<6; ++i ) m_iomuxFuncs[i] = { nullptr, nullptr, "- -" };
}
Esp32Pin::~Esp32Pin() {}

void Esp32Pin::initialize()
{
    //Pin::setLabelText( m_pinLabel );
    //Pin::setLabelColor( QColor( 250, 250, 200 ) );

    //m_isAnalog = false;
    ////m_portState = false;

    //double vdd = 3.3; //m_port->getMcu()->vdd();
    //m_outHighV = vdd;
    //m_inpHighV = vdd/2;
    //m_inpLowV  = vdd/2;
    //

    IoPin::initialize();
}

void Esp32Pin::updateStep()
{
    IoPin::updateStep();
    Pin::setLabelText( m_iomuxFuncs[m_iomuxIndex].label );
    //Simulator::self()->remFromUpdateList( this );
}

void Esp32Pin::stamp()
{
    //m_alternate = false;
    //m_analog = false;
    //m_pull = false;

    m_pullUp   = 0;
    m_pullDown = 0;
    m_inputEn  = 0;
    setPinMode( input );

    m_iomuxIndex = -1;
    selectIoMuxFunc( 0 );

    //setPull( true );
    //updateStep();

    //update();

    IoPin::stamp();
}

void Esp32Pin::voltChanged()
{
    bool oldState = m_inpState;
    bool newState = IoPin::getInpState();

    if( oldState == newState ) return;

    // while( m_arena->qemuAction )        // Wait for previous action executed
    // {
    //     ; /// TODO: add timeout
    // }
    // m_arena->data8 = m_port-1;
    // m_arena->mask8 = m_number;
    // m_arena->data16 = newState;
    // m_arena->qemuAction = SIM_GPIO_IN;
}

void Esp32Pin::setPinMode( pinMode_t mode )
{
    IoPin::setPinMode( mode );
    changeCallBack( this, mode == input );
}

//bool Esp32Pin::setAlternate( bool a ) // If changing to Not Alternate, return false
//{
//    if( m_alternate == a ) return true;
//    m_alternate = a;
//    if( a ) qDebug() << "Esp32Pin::setAlternate" << this->m_id;
//    return a;
//}
//
//void Esp32Pin::setAnalog( bool a ) /// TODO: if changing to Not Analog, return false
//{
//    if( m_analog == a ) return;
//    m_analog = a;
//}

void Esp32Pin::setPortState(  bool high ) // Set output from Port register
{
    //if( m_alternate ) return;
    setPinState( high );
}

void Esp32Pin::setOutState( bool high ) // Set output from Alternate (peripheral)
{
    //if( m_alternate )
        setPinState( high );
}

void Esp32Pin::scheduleState( bool high, uint64_t time )
{
    //if( m_alternate )
        IoPin::scheduleState( high, time );
}

void Esp32Pin::setPinState( bool high ) // Set Output to Hight or Low
{
    m_outState = m_nextState = high;
    //if( m_pinMode < openCo  || m_stateZ ) return;

    if( m_inverted ) high = !high;

    switch( m_pinMode )
    {
        case undef_mode: return;
        case input:
            /// if( m_pull ){
            ///     m_outVolt = high ? m_outHighV : m_outLowV;
            ///     ePin::stampCurrent( m_outVolt*m_pullAdmit );
            /// }
            break;
        case output:
            m_outVolt = high ? m_outHighV : m_outLowV;
            ePin::stampCurrent( m_outVolt*m_admit );
            break;
        case openCo:
            m_gndAdmit = high ? 1/1e8 : 1/m_outputImp;
            updtState();
            break;
        default: return;
    }
}

void Esp32Pin::setIoMuxFuncs( QList<funcPin> functions ) // Set IO_MUX functions for this pad
{
    for( int i=0; i<6; ++i )
    {
        funcPin fp = functions.at(i);
        if( fp.label.isEmpty() ) fp.label = m_pinLabel;

        m_iomuxFuncs[i] = fp;
    }
}

void Esp32Pin::selectIoMuxFunc( uint8_t func ) // Select IO_MUX function
{
    if( func > 5 ){
        qDebug() << this->pinId() << "Selected func ERROR"<< func;
        return;
    }
    if( m_iomuxIndex < 6 ){
        if( m_iomuxFuncs[m_iomuxIndex].pinPointer )
            *m_iomuxFuncs[m_iomuxIndex].pinPointer = m_dummyPin;

        QemuModule* mod = m_iomuxFuncs[m_iomuxIndex].module;
        if( mod ) mod->connected( false );
    }
    if( m_iomuxFuncs[func].label == "GPIO" ) m_iomuxFuncs[func].label = m_pinLabel;


    Simulator::self()->addToUpdateList( this ); /// FIXME

    if( m_iomuxFuncs[func].pinPointer )
    {
        //qDebug() << this->pinId() << "Selected func"<< func << m_iomuxPin[func].label;
        *m_iomuxFuncs[func].pinPointer = this;

        QemuModule* mod = m_iomuxFuncs[func].module;
        if( mod ) mod->connected( true );
    }
    if( m_iomuxFuncs[func].pinPointer || m_iomuxFuncs[func].label == m_pinLabel ){
        Pin::setLabelColor( QColor( 255, 255, 100 ) );
    }else{
        Pin::setLabelColor( QColor( 100, 100, 100 ) );
    }
    m_iomuxIndex = func;
    //qDebug() << this->pinId() << "Selected func"<< func << m_iomuxFuncs[func].label;
    //update();
}

void Esp32Pin::setMatrixFunc( uint16_t val, funcPin func ) // Set Function for GPIO Matrix: index=2
{
    //qDebug() << this->pinId() << "Matrix function"<< func.label<< (val & 0b111000000000);
    m_iomuxFuncs[2] = func;
    if( m_iomuxIndex == 2 ) selectIoMuxFunc( 2 );
}

void Esp32Pin::writeIoMuxReg( uint16_t value )
{
    // Sleep bits 0-6
    // PD bit 7
    // PU bit 8
    // IE bit 9
    // Drive bits 10-11
    // function bits 12-14

    uint8_t puld = (value >> 7) & 1;
    cfgPulldo( puld );

    uint8_t pulu = (value >> 8) & 1;
    cfgPullup( pulu );

    m_inputEn = (value >> 9) & 1;

    uint8_t func = (value >> 12) & 7;
    if( m_iomuxIndex == func ) return;
    selectIoMuxFunc( func );
}

void Esp32Pin::cfgPullup( uint8_t on )
{
    if( m_pullUp == on ) return;
    m_pullUp = on;
    double pullup = on ? m_pullResist : 0;
    IoPin::setPullup( pullup );
}

void Esp32Pin::cfgPulldo( uint8_t on )
{
    if( m_pullDown == on ) return;
    m_pullDown = on;
    double pulldo = on ? m_pullResist : 0;
    IoPin::setPulldown( pulldo );
}

void Esp32Pin::writePinReg( uint32_t value )
{

}

void Esp32Pin::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    if( !isVisible() ) return;
    Pin::paint( p, o, w );

    if( !m_pullUp && !m_pullDown ) return;
    if( m_pinMode > openCo ) return;

    // Draw pullUp/Down dot

    if( m_pullUp ) p->setBrush( QColor( 255, 180,   0 ) );
    else           p->setBrush( QColor(   0, 180, 255 ) );

    QPen pen = p->pen();
    pen.setWidthF( 0 );
    p->setPen(pen);
    int start = (m_length > 4) ? m_length-4.5 : 3.5;
    QRectF rect( start+0.6,-1.5, 3, 3 );
    p->drawEllipse(rect);
}
