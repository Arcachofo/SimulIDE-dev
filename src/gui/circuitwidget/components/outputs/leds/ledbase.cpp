/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "ledbase.h"
#include "circuitwidget.h"
#include "connector.h"
#include "simulator.h"
#include "e-node.h"
#include "pin.h"

#include "doubleprop.h"
#include "stringprop.h"
#include "boolprop.h"

eNode LedBase::m_gndEnode("");

LedBase::LedBase( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eLed( id )
{
    m_graphical = true;
    m_grounded  = false;
    m_intensity = 0;

    m_enumUids = QStringList()
        << "Yellow"
        << "Red"
        << "Green"
        << "Blue"
        << "Orange"
        << "Purple"
        << "White";

    m_enumNames = QStringList()
        << tr("Yellow")
        << tr("Red")
        << tr("Green")
        << tr("Blue")
        << tr("Orange")
        << tr("Purple")
        << tr("White");

    m_color = QColor( Qt::black );
    setColorStr("Yellow");

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new StrProp <LedBase>("Color"   , tr("Color")   ,"", this, &LedBase::colorStr, &LedBase::setColorStr,0,"enum" ),
new BoolProp<LedBase>("Grounded", tr("Grounded"),"", this, &LedBase::grounded, &LedBase::setGrounded),
    }, groupNoCopy} );

    addPropGroup( { tr("Electric"), {
new DoubProp<LedBase>("Threshold" , tr("Forward Voltage"),"V", this, &LedBase::threshold,  &LedBase::setThreshold ),
new DoubProp<LedBase>("MaxCurrent", tr("Max Current")    ,"A", this, &LedBase::maxCurrent, &LedBase::setMaxCurrent ),
new DoubProp<LedBase>("Resistance", tr("Resistance")     ,"Ω", this, &LedBase::res,        &LedBase::setRes ),
    }, groupNoCopy} );
}
LedBase::~LedBase(){}

void LedBase::initialize()
{
    m_crashed = false;
    m_warning = false;

    if( m_grounded ) m_ePin[1]->setEnode( &m_gndEnode );

    eLed::initialize();
    update();
}

void LedBase::updateStep()
{
    uint32_t intensity = m_intensity;
    eLed::updateBright();

    if( overCurrent() > 1.5 )
    {
        m_warning = true;
        m_crashed = overCurrent() > 2;
        update();
    }else{
        if( m_warning ) update();
        m_warning = false;
        m_crashed = false;
    }
    if( intensity != m_intensity ) update();
    if( m_changed )
    {
        m_changed = false;
        voltChanged();
    }
}

void LedBase::setGrounded( bool grounded )
{
    if( grounded == m_grounded ) return;
    m_grounded = grounded;

    if( Simulator::self()->isRunning() )  CircuitWidget::self()->powerCircOff();

    Pin* pin1 = static_cast<Pin*>(m_ePin[1]);
    pin1->setEnabled( !grounded );
    pin1->setVisible( !grounded );
    if( grounded ) pin1->removeConnector();
    else           pin1->setEnode( NULL );
}

void LedBase::setColorStr( QString color )
{
    int ledColor = getEnumIndex( color );
    m_ledColor = (LedColor)ledColor;
    double thr;
    switch( m_ledColor ) {
        case yellow: thr = 2.4; break;
        case red:    thr = 1.8; break;
        case green:  thr = 3.5; break;
        case blue:   thr = 3.6; break;
        case orange: thr = 2.0; break;
        case purple: thr = 3.5; break;
        case white:  thr = 4.0; break;
        default:     thr = 2.4; break;
    }
    eLed::setThreshold( thr );
    if( m_showVal && (m_showProperty == "Color") )
        setValLabelText( m_enumNames.at( ledColor ) );
}

QColor LedBase::getColor( LedColor c, int intensity )
{
    int overBight = 100;
    QColor color;

    if( intensity > 25 )
    {
        intensity += 15;       // Set a Minimun Bright
        if( intensity > 255 )
        {
            overBight += intensity-255;
            intensity = 255;
    }   }
    switch( c ) {
        case yellow: color = QColor( intensity, intensity,               overBight*2/3 ); break;
        case red:    color = QColor( intensity, intensity/4+overBight/2, overBight/2 );   break;
        case green:  color = QColor( overBight, intensity,               intensity*2/3 ); break;
        case blue:   color = QColor( overBight, intensity*2/3,           intensity );     break;
        case orange: color = QColor( intensity, intensity*2/3,           overBight );     break;
        case purple: color = QColor( intensity, intensity/4+overBight/2, intensity );     break;
        case white:  color = QColor( intensity, intensity,               intensity );     break;
    }
    return color;
}
void LedBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen( Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    QColor color;

    if( m_warning/*m_current > m_maxCurrent*1.2*/ ) // Led overcurrent
    {
        p->setBrush( QColor( 255, 150, 0 ) );
        color = QColor( Qt::red );
        pen.setColor( color );
    }
    if( m_crashed )  // Led extreme overcurrent
    {
        p->setBrush( Qt::white );
        color = QColor( Qt::white );
        pen.setColor( color );
    }else{
        color = getColor( m_ledColor, m_intensity );
    }
    p->setPen( pen );
    drawBackground( p );
    
    pen.setColor( color );
    pen.setWidth( 2 );
    p->setPen( pen );
    p->setBrush( color );

    drawForeground( p );
}
