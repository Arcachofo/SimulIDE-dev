/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QtMath>
#include <QPainter>

#include "stepper.h"
#include "simulator.h"
#include "connector.h"

#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

Component*  Stepper::construct( QObject* parent, QString type, QString id )
{ return new Stepper( parent, type, id ); }

LibraryItem* Stepper::libraryItem()
{
    return new LibraryItem(
        tr("Stepper"),
        "Motors",
        "steeper.png",
        "Stepper",
        Stepper::construct );
}

Stepper::Stepper( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( (id+"-eElement") )
       , m_resA1( (id+"-eEresistorA1") )
       , m_resA2( (id+"-eEresistorA2") )
       , m_resB1( (id+"-eEresistorB1") )
       , m_resB2( (id+"-eEresistorB2") )
       , m_pinA1( 180, QPoint(-72,-32), id+"-PinA1", 0, this )
       , m_pinA2( 180, QPoint(-72, 16), id+"-PinA2", 0, this )
       , m_pinCo( 180, QPoint(-72, 0 ), id+"-PinCo", 0, this )
       , m_pinB1( 180, QPoint(-72,-16), id+"-PinB1", 0, this )
       , m_pinB2( 180, QPoint(-72, 32), id+"-PinB2", 0, this )
       , m_ePinA1Co( (id+"-ePinA1Co"), 0 )
       , m_ePinA2Co( (id+"-ePinA2Co"), 0 )
       , m_ePinB1Co( (id+"-ePinB1Co"), 0 )
       , m_ePinB2Co( (id+"-ePinB2Co"), 0 )
{
    m_graphical = true;
    m_area = QRectF( -64, -50, 114, 100 );
    m_color = QColor( 50, 50, 70 );
    m_bipolar = false;
    m_ang  = 0;
    m_Ppos = 4;
    m_steps = 32;
    m_stpang = 360*8/m_steps;
        
    m_pinA1.setLabelText( " A+" );
    m_pinA2.setLabelText( " A-" );
    m_pinCo.setLabelText( " Co" );
    m_pinB1.setLabelText( " B+" );
    m_pinB2.setLabelText( " B-" );
    
    m_resA1.setEpin( 0, &m_pinA1 );
    m_resA1.setEpin( 1, &m_ePinA1Co );
    m_resA2.setEpin( 0, &m_pinA2 );
    m_resA2.setEpin( 1, &m_ePinA2Co );
    m_resB1.setEpin( 0, &m_pinB1 );
    m_resB1.setEpin( 1, &m_ePinB1Co );
    m_resB2.setEpin( 0, &m_pinB2 );
    m_resB2.setEpin( 1, &m_ePinB2Co );
    
    setRes( 100 );
    setShowId( true );
    setLabelPos(-32,-62, 0);
    setValLabelPos(-14, 36, 0);

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new BoolProp<Stepper>( "Bipolar"   , tr("Bipolar")           ,""      , this, &Stepper::bipolar, &Stepper::setBipolar ),
new IntProp <Stepper>( "Steps"     , tr("Steps per Rotation"),tr("_Teeth"), this, &Stepper::steps,   &Stepper::setSteps ),
new DoubProp<Stepper>( "Resistance", tr("Resistance")        ,"Ω"     , this, &Stepper::res,     &Stepper::setRes )
    },0} );
}
Stepper::~Stepper(){}

void Stepper::stamp()
{
    eNode* enode = m_pinCo.getEnode();// Register for changes callback
    if( enode )
    {
        m_ePinA1Co.setEnode( enode );
        m_ePinA2Co.setEnode( enode );
        m_ePinB1Co.setEnode( enode );
        m_ePinB2Co.setEnode( enode );
    }
    m_pinA1.changeCallBack( this );// Register for changes callback
    m_pinA2.changeCallBack( this );// Register for changes callback
    m_pinB1.changeCallBack( this );// Register for changes callback
    m_pinB2.changeCallBack( this );// Register for changes callback
    m_pinCo.changeCallBack( this );// Register for changes callback
}

void Stepper::updateStep()
{
    if( m_changed )
    {
        m_changed = false;
        m_resA1.setRes( m_res );
        m_resA2.setRes( m_res );
        m_resB1.setRes( m_res );
        m_resB2.setRes( m_res );
    }
    update();
}

void Stepper::voltChanged()
{
    double voltCom = m_pinCo.getVoltage();
    double phaseA = ( m_pinA1.getVoltage()-voltCom )-( m_pinA2.getVoltage()-voltCom );
    double phaseB = ( m_pinB1.getVoltage()-voltCom )-( m_pinB2.getVoltage()-voltCom );
    
    if     ( phaseA > 1 ) phaseA = 1;
    else if( phaseA <-1 ) phaseA =-1;
    else                  phaseA = 0;
    
    if     ( phaseB > 1 ) phaseB = 1;
    else if( phaseB <-1 ) phaseB =-1;
    else                  phaseB = 0;

    int delta = 0;
    if( (qFabs(phaseA)+qFabs(phaseB)) > 0 ) // nosense algoritm.. just works
    {
        int ca = 4; 
        int cb =-1;
        if( phaseA ==-1 ) ca = 0;
        if( phaseA == 0 ) cb = 2;
        if( phaseA == 1 ) cb = 1;
        
        int newPos = ca+cb*phaseB;
        
        delta = newPos-m_Ppos;
        if( delta > 4 ) delta = delta-8;
        if( delta <-4 ) delta = delta+8;
        
        m_Ppos += delta;
    }
    else{                       // avoid keep in half step when no input
        delta = m_Ppos;
        m_Ppos = (m_Ppos/2)*2;
        delta = m_Ppos-delta;
    }
    m_ang  += delta*m_stpang;

    if (m_ang < 0)      m_ang += 360*16;
    if (m_ang > 360*16) m_ang -= 360*16;
    if (m_Ppos < 0)     m_Ppos += 8;
    if (m_Ppos > 7)     m_Ppos -= 8;
}

void Stepper::setSteps( int steps ) //" 4, 8,16,32"
{
    m_steps = steps/4;
    m_steps *= 4;
    if( m_steps < 4 ) m_steps = 4;
    m_stpang = 360*8/m_steps;
    m_ang  = 0;
    m_Ppos = 4;
    update();
}

void Stepper::setRes( double res )
{
    if( res < 1e-12 ) res = 1e-12;
    if( m_res == res ) return;
    m_res = res;
    m_changed = true;
}

void Stepper::setBipolar( bool bi )
{
    m_bipolar = bi;
    m_pinCo.removeConnector();
    m_pinCo.setVisible( !bi );
}

void Stepper::remove()
{
    m_pinA1.removeConnector();
    m_pinA2.removeConnector();
    m_pinCo.removeConnector();
    m_pinB1.removeConnector();
    m_pinB2.removeConnector();
    
    Component::remove();
}

void Stepper::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    p->setRenderHint( QPainter::Antialiasing );
    Component::paint( p, option, widget );

    //p->setBrush( QColor(250, 210, 230) );
    p->drawRoundRect(-64,-40, 25, 80 );

    p->setBrush( QColor(50, 70, 100) ); 
    p->drawRoundRect(-48,-48, 96, 96 );

    p->setPen( QColor(0, 0, 0) );
    p->setBrush( QColor(255, 255, 255) );
    p->drawEllipse(-37,-37, 74, 74 );

    p->setPen ( QColor(255, 255, 255) );
    QRectF rectangle( -42.0,-42.0, 84.0, 84.0 );
    for ( int i = 0; i< 360*16; i += m_stpang*2 )
        p->drawPie( rectangle, i+m_stpang*5/6, m_stpang/3 );

    p->setPen ( QColor(0, 0, 0) );
    p->setBrush( QColor(50, 70, 100) );
    p->drawPie(-33,-33, 66, 66, m_ang-m_stpang*3/4, m_stpang*3/2 );

    //p->setBrush( QColor(50, 70, 100) );
    p->drawEllipse(-25,-25, 50, 50);
}
