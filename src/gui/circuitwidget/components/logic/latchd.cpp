/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "latchd.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "simulator.h"
#include "circuit.h"
#include "iopin.h"

#include "stringprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* LatchD::construct( QObject* parent, QString type, QString id )
{ return new LatchD( parent, type, id ); }

LibraryItem* LatchD::libraryItem()
{
    return new LibraryItem(
        tr( "Latch" ),
        "Memory",
        "subc.png",
        "LatchD",
        LatchD::construct );
}

LatchD::LatchD( QObject* parent, QString type, QString id )
      : LogicComponent( parent, type, id )
{
    m_width  = 4;
    m_height = 10;
    m_area = QRect(-m_width*8/2,-m_height*8/2, m_width*8, m_height*8 );
    
    m_tristate = true;
    createOePin("IR13OE", id+"-Pin_outEnable");

    m_clkPin = new IoPin( 180, QPoint( -24,0 ), m_id+"-Pin_clock", 0, this, input );
    m_clkPin->setLabelColor( QColor( 0, 0, 0 ) );
    LatchD::setTrigger( InEnable );

    m_channels = 0;
    setChannels( 8 );

    addPropGroup( { tr("Main"), {
new IntProp <LatchD>("Channels" , tr("Size")        ,"_Channels", this, &LatchD::channels,   &LatchD::setChannels,0,"uint" ),
new StrProp <LatchD>("Trigger"  , tr("Trigger Type"),""         , this, &LatchD::triggerStr, &LatchD::setTriggerStr,0,"enum" ),
new BoolProp<LatchD>("Tristate" , tr("Tristate")    ,""         , this, &LatchD::tristate,   &LatchD::setTristate ),
    }, groupNoCopy } );
    addPropGroup( { tr("Electric"), IoComponent::inputProps()
+QList<ComProperty*>({new BoolProp<LatchD>( "Invert_Inputs", tr("Invert Inputs"),"", this, &LatchD::invertInps, &LatchD::setInvertInps,propNoCopy )})
                    +IoComponent::outputProps()+IoComponent::outputType(),0 } );
    addPropGroup( { tr("Edges"), IoComponent::edgeProps(),0 } );
}
LatchD::~LatchD(){}

void LatchD::stamp()
{
    if( m_trigger != Clock )
    { for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this ); }
    LogicComponent::stamp();
}

void LatchD::voltChanged()
{
    updateOutEnabled();
    updateClock();
    if( m_clkState == Clock_Allow )
    {
        m_nextOutVal = 0;
        for( uint i=0; i<m_outPin.size(); ++i )
            if( m_inPin[i]->getInpState() ) m_nextOutVal |= 1<<i;
    }
    sheduleOutPuts( this );
}

void LatchD::setChannels( int channels )
{
    if( channels == m_channels ) return;
    if( channels < 1 ) return;
    
    setNumInps( channels, "D");
    setNumOuts( channels );
    m_channels = channels;

    m_height = channels+2;
    int y = -(m_height/2)*8+8;

    for( int i=0; i<channels; i++ )
    {
        m_inPin[i]->setY( y+i*8 );
        m_outPin[i]->setY( y+i*8 );
    }
    y += channels*8;
    m_clkPin->setY( y );
    m_oePin->setY( y );

    updateSize();
}

void LatchD::setTristate( bool t )
{
    LogicComponent::setTristate( t );
    updateSize();
}

void LatchD::setTrigger( trigger_t trigger )
{
    eClockedDevice::setTrigger( trigger );
    updateSize();
}

void LatchD::updateSize()
{
    int height = m_height;
    if( !m_tristate && (m_trigger == None) ) height--;
    m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, height*8 );
    Circuit::self()->update();
}
