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

#define tr(str) simulideTr("LedBase",str)

eNode LedBase::m_gndEnode("");
int   LedBase::m_overBright = 0;

LedBase::LedBase( QString type, QString id )
       : Component( type, id )
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
        new StrProp <LedBase>("Color", tr("Color"), ""
                             , this, &LedBase::colorStr, &LedBase::setColorStr,0,"enum" ),

        new BoolProp<LedBase>("Grounded", tr("Grounded"), ""
                             , this, &LedBase::grounded, &LedBase::setGrounded, propNoCopy ),
    }, 0} );

    addPropGroup( { tr("Electric"), {
        new DoubProp<LedBase>("Threshold", tr("Forward Voltage"), "V"
                             , this, &LedBase::threshold, &LedBase::setThreshold ),

        new DoubProp<LedBase>("MaxCurrent", tr("Max Current"), "A"
                             , this, &LedBase::maxCurrent, &LedBase::setMaxCurrent ),

        new DoubProp<LedBase>("Resistance", tr("Resistance"), "Ω"
                             , this, &LedBase::res, &LedBase::setRes ),
    }, 0} );

    setPropStr("MaxCurrent", "30 mA");
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

void LedBase::setColorStr( QString foreColor )
{
    int ledColor = getEnumIndex( foreColor );
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

QColor LedBase::getColor(LedColor c, int bright )
{
    m_overBright = 0;
    int secL = bright/3;
    int secH = bright/2;
    int secX = bright*2/3;
    QColor foreColor;

    if( bright > 255 )
    {
        m_overBright = (bright-255);
        secL += m_overBright;
        bright = 255;
    }
    switch( c ) {
        case yellow: foreColor = QColor( bright, bright, secL ); break;
        case red:    foreColor = QColor( bright, secH  , secH ); break;
        case green:  foreColor = QColor( secL  , bright, secL ); break;
        case blue:   foreColor = QColor( secH  , secH  , bright ); break;
        case orange: foreColor = QColor( bright, secX  , secL ); break;
        case purple: foreColor = QColor( bright, secL  , bright ); break;
        case white:  foreColor = QColor( bright, bright, bright ); break;
    }
    return foreColor;
}
void LedBase::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    QPen pen( Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    QColor foreColor;
    QColor backColor = QColor(0,0,0);

    if( m_warning/*m_current > m_maxCurrent*1.2*/ ) // Led overcurrent
    {
        p->setBrush( QColor( 255, 150, 0 ) );
        foreColor = QColor( Qt::red );
        pen.setColor( foreColor );
    }
    if( m_crashed )  // Led extreme overcurrent
    {
        p->setBrush( Qt::white );
        foreColor = QColor( Qt::white );
        pen.setColor( foreColor );
    }else{
        foreColor = getColor( m_ledColor, m_intensity );
        int over = m_overBright*2;
        backColor = QColor( over, over, m_overBright );
    }
    pen.setColor( backColor );
    p->setPen( pen );
    p->setBrush( backColor );
    drawBackground( p );
    
    pen.setColor( foreColor );
    pen.setWidth( 2 );
    p->setPen( pen );
    p->setBrush( foreColor );
    drawForeground( p );

    Component::paintSelected( p );
}
