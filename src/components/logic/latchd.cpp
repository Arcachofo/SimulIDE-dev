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

#define tr(str) simulideTr("LatchD",str)

Component* LatchD::construct( QString type, QString id )
{ return new LatchD( type, id ); }

LibraryItem* LatchD::libraryItem()
{
    return new LibraryItem(
        tr( "Latch" ),
        "Memory",
        "subc.png",
        "LatchD",
        LatchD::construct );
}

LatchD::LatchD( QString type, QString id )
      : LogicComponent( type, id )
{
    m_width  = 4;
    m_height = 10;
    m_area = QRect(-(m_width*8/2),-(m_height*8/2), m_width*8, m_height*8 );
    
    m_useReset = false;
    m_tristate = true;
    createOePin("IR00OE", id+"-Pin_outEnable");

    m_clkPin = new IoPin( 180, QPoint(-24, 0 ), m_id+"-Pin_clock", 0, this, input );
    m_clkPin->setLabelColor( QColor( 0, 0, 0 ) );

    m_resetPin = new IoPin( 180, QPoint(-24, 8 ), m_id+"-Pin_reset", 0, this, input );
    setupPin( m_resetPin,"L03RST");
    m_resetPin->setVisible( false );
    m_otherPin.emplace_back( m_resetPin );

    m_channels = 0;
    setChannels( 8 );
    LatchD::setTrigger( InEnable );

    addPropGroup( { tr("Main"), {
        new IntProp <LatchD>("Channels", tr("Size"),"_bits"
                            , this, &LatchD::channels, &LatchD::setChannels, propNoCopy,"uint" ),

        new StrProp <LatchD>("Trigger", tr("Trigger Type"), LogicComponent::m_triggerList
                            , this, &LatchD::triggerStr, &LatchD::setTriggerStr, propNoCopy,"enum" ),

        new BoolProp<LatchD>("UseRS", tr("Reset Pin"),""
                            , this, &LatchD::pinReset, &LatchD::setPinReset, propNoCopy ),
    }, groupNoCopy } );

    addPropGroup( { tr("Electric"),
        IoComponent::inputProps()
        +QList<ComProperty*>({
            new BoolProp<LatchD>("Invert_Inputs", tr("Invert Inputs"),""
                                , this, &LatchD::invertInps, &LatchD::setInvertInps,propNoCopy )
        })
        + IoComponent::outputProps()
        + IoComponent::outputType()
        +QList<ComProperty*>({
            new BoolProp<LatchD>("Tristate", tr("Tristate"),""
                                , this, &LatchD::tristate, &LatchD::setTristate ),
        })
    ,0 } );

    addPropGroup( { tr("Timing"), IoComponent::edgeProps(),0 } );
}
LatchD::~LatchD(){}

void LatchD::stamp()
{
    if( m_trigger != Clock )
    { for( uint i=0; i<m_inPin.size(); ++i ) m_inPin[i]->changeCallBack( this ); }

    m_resetPin->changeCallBack( this );
    LogicComponent::stamp();
}

void LatchD::voltChanged()
{
    if( m_tristate ) updateOutEnabled();
    updateClock();

    if( m_resetPin->getInpState() ) m_nextOutVal = 0;
    else if( m_clkState == Clock_Allow )
    {
        m_nextOutVal = 0;
        for( uint i=0; i<m_outPin.size(); ++i )
            if( m_inPin[i]->getInpState() ) m_nextOutVal |= 1<<i;
    }
    scheduleOutPuts( this );
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

void LatchD::setPinReset( bool r )
{
    m_useReset = r;
    m_resetPin->setVisible( r );
    if( !r ) m_resetPin->removeConnector();
    updateSize();
}
void LatchD::updateSize()
{
    int height = m_height-1;
    int l = m_useReset ? 1 : 0;
    if( m_trigger != None ) l++;
    int d = m_tristate ? 1 : 0;
    height = (l > d) ? height+l : height+d;

    int y = -(m_height/2)*8;
    m_area = QRect( -(m_width/2)*8, y, m_width*8, height*8 );
    y += m_channels*8+8;
    m_clkPin->setY( y );
    m_oePin->setY( y );
    if( m_trigger != None ) y += 8;
    m_resetPin->setY( y );
    Circuit::self()->update();
}
