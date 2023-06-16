/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "csource.h"
#include "simulator.h"
#include "circuitwidget.h"
#include "itemlibrary.h"
#include "propdialog.h"
#include "pin.h"

#include "doubleprop.h"
#include "boolprop.h"

Component* Csource::construct( QObject* parent, QString type, QString id )
{ return new Csource( parent, type, id ); }

LibraryItem* Csource::libraryItem()
{
    return new LibraryItem(
        tr("Controlled Source"),
        "Sources",
        "csource.png",
        "Csource",
        Csource::construct );
}

Csource::Csource( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eResistor( id )
{
    m_area = QRect(-16,-16, 32, 32 );

    m_pin.resize(4);
    // Control Pins
    m_pin[0] = new Pin( 180, QPoint(-24,-8 ), id+"-cpPin", 0, this);
    m_pin[1] = new Pin( 180, QPoint(-24, 8 ), id+"-cmPin", 1, this);
    m_pin[0]->setFontSize( 5 );
    m_pin[0]->setLabelText("➕");
    m_pin[0]->setLabelColor( Qt::red );
    m_pin[1]->setFontSize( 5 );
    m_pin[1]->setLabelText("➖");
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];
    // Source Pins
    m_pin[2] = new Pin(  90, QPoint( 0,-20 ), id+"-s1Pin", 0, this);
    m_pin[3] = new Pin( 270, QPoint( 0, 20 ), id+"-s2Pin", 1, this);
    m_pin[2]->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
    m_pin[3]->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

    m_controlPins = true;
    m_currSource  = true;
    m_currControl = false;
    m_volt = 5;
    m_curr = 1;
    m_gain = 1;

    setLabelPos( 4,-28, 0 );
    setValLabelPos( 4, 18, 0 ); // x, y, rot

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new BoolProp<Csource>( "CurrSource"  , tr("Current Source")    ,"" , this, &Csource::currSource , &Csource::setCurrSource ),
new BoolProp<Csource>( "CurrControl" , tr("Current Controlled"),"" , this, &Csource::currControl, &Csource::setCurrControl, propNoCopy ),
new BoolProp<Csource>( "Control_Pins", tr("Use Control Pins")  ,"" , this, &Csource::controlPins, &Csource::setControlPins, propNoCopy ),
new DoubProp<Csource>( "Gain"        , tr("Gain")              ,"" , this, &Csource::gain       , &Csource::setGain ),
new DoubProp<Csource>( "Voltage"     , tr("Voltage")           ,"V", this, &Csource::volt       , &Csource::setVolt ),
new DoubProp<Csource>( "Current"     , tr("Current")           ,"A", this, &Csource::outCurrent , &Csource::setOutCurrent )
    }, 0 } );
}
Csource::~Csource() {}

void Csource::stamp()
{
    if( m_currControl ) m_admit = 1/cero_doub;
    else                m_admit = cero_doub;
    eResistor::stamp();

    m_pin[2]->setEnodeComp( m_pin[3]->getEnode() );
    m_pin[3]->setEnodeComp( m_pin[2]->getEnode() );
    m_pin[2]->createCurrent();
    m_pin[3]->createCurrent();

    m_changed = true;
    updateStep();
}

void Csource::voltChanged()
{
    double volt = m_pin[0]->getVoltage() - m_pin[1]->getVoltage();
    double curr = volt;

    if( m_currControl )              curr *= m_admit;
    if( !m_currSource && curr != 0 ) curr /= cero_doub;

    curr *= m_gain;
    m_pin[2]->stampCurrent( curr );
    m_pin[3]->stampCurrent(-curr );
}

void Csource::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    udtProperties();

    m_pin[0]->setEnabled( m_controlPins );
    m_pin[0]->setVisible( m_controlPins );
    m_pin[1]->setEnabled( m_controlPins );
    m_pin[1]->setVisible( m_controlPins );

    if( m_currControl )
    {
        m_admit = 1/cero_doub;
        m_pin[0]->setLabelText( "" );
        m_pin[1]->setLabelText( "" );
    }else{
        m_admit = cero_doub;
        m_pin[0]->setLabelText("➕");
        m_pin[1]->setLabelText("➖");
    }
    eResistor::stampAdmit();

    if( m_currSource )
    {
        m_pin[2]->stampAdmitance( 0 );
        m_pin[3]->stampAdmitance( 0 );
    }else{
        m_pin[2]->stampAdmitance( 1/cero_doub );
        m_pin[3]->stampAdmitance( 1/cero_doub );
    }

    if( !m_controlPins )
    {
        m_pin[0]->removeConnector();
        m_pin[1]->removeConnector();

        if( m_currSource )
        {
            m_pin[2]->stampCurrent( m_curr );
            m_pin[3]->stampCurrent(-m_curr );
        }else{
            m_pin[2]->stampCurrent( m_volt/cero_doub );
            m_pin[3]->stampCurrent(-m_volt/cero_doub );
        }
    }
    else voltChanged();

    bool connected = m_pin[0]->isConnected() && m_pin[1]->isConnected();
    m_pin[0]->changeCallBack( this, connected && m_controlPins );
    m_pin[1]->changeCallBack( this, connected && m_controlPins );
    update();
}

void Csource::setGain( double g )
{
    m_gain = g;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Csource::setVolt( double volt )
{
    if( volt < 0 ) volt = 0;
    m_volt = volt;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Csource::setOutCurrent( double c )
{
    if( c < 0 ) c = 0;
    m_curr = c;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Csource::setCurrSource( bool c )
{
    m_currSource = c;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

 void Csource::setCurrControl( bool c )
 {
     m_currControl = c;
     m_changed = true;
     if( !Simulator::self()->isRunning() ) updateStep();
 }

void Csource::setControlPins( bool set )
{
    m_controlPins = set;
    m_changed = true;
    if( !Simulator::self()->isRunning() ) updateStep();
}

void Csource::udtProperties()
{
    if( !m_propDialog ) return;
    m_propDialog->showProp("Voltage", !m_controlPins && !m_currSource );
    m_propDialog->showProp("Current", !m_controlPins && m_currSource);
    m_propDialog->showProp("CurrControl", m_controlPins );
    m_propDialog->showProp("Gain", m_controlPins );
}

void Csource::slotProperties()
{
    Component::slotProperties();
    udtProperties();
}

void Csource::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPainterPath path;
    QVector<QPointF> points;
    points << QPointF(-8, 0 )
           << QPointF( 0,-13 )
           << QPointF( 8, 0 )
           << QPointF( 0, 13 );

    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();

    QPen pen = p->pen();

    if( m_controlPins )
    {
        pen.setWidth(1);
        p->setPen(pen);
        p->drawRect( m_area );
    }
    pen.setWidth(2);
    p->setPen(pen);
    p->drawPath( path );

    pen.setWidth(1);
    p->setPen(pen);
    if( m_currSource )
    {
        p->drawLine( 0,-5, 0, 5 );
        p->drawLine(-2, 2, 0, 5 );
        p->drawLine( 2, 2, 0, 5 );
    }else{
        p->drawLine(-2,-4, 2,-4 );
        p->drawLine( 0,-6, 0,-2 );
        p->drawLine(-2, 4, 2, 4 );
    }
    if( m_currControl && m_controlPins )
    {
        pen.setWidthF(0.6);
        p->setPen(pen);
        p->drawLine(-12,-7,-12, 7 );
        p->drawLine(-13, 5,-12, 7 );
        p->drawLine(-11, 5,-12, 7 );
    }
}
