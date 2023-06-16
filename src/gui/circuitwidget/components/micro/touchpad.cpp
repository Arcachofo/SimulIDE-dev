/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QGraphicsProxyWidget>
#include <QToolButton>

#include "touchpad.h"
#include "simulator.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "e-node.h"

#include "doubleprop.h"
#include "boolprop.h"
#include "intprop.h"

Component* TouchPad::construct( QObject* parent, QString type, QString id )
{ return new TouchPad( parent, type, id ); }

LibraryItem* TouchPad::libraryItem()
{
    return new LibraryItem(
        tr( "TouchPad (Resistive)" ),
        "Perifericals",
        "touch.png",
        "TouchPadR",
        TouchPad::construct);
}

TouchPad::TouchPad( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
     , m_resXA( id+"-resXA" )
     , m_resXB( id+"-resXB" )
     , m_resYA( id+"-resYA" )
     , m_resYB( id+"-resYB" )
     , m_resTouch( id+"-resTouch" )
     , m_ePinXA( id+"-ePinXA", 1 )
     , m_ePinXB( id+"-ePinXB", 1 )
     , m_ePinYA( id+"-ePinYA", 1 )
     , m_ePinYB( id+"-ePinYB", 1 )
     , m_ePinTA( id+"-ePinTA", 1 )
     , m_ePinTB( id+"-ePinTB", 1 )
{
    m_eNodeX = NULL;
    m_eNodeY = NULL;

    m_RxMin = 100;
    m_RxMax = 500;
    m_RyMin = 100;
    m_RyMax = 500;

    m_transparent = false;

    m_pin.resize(4);
    m_vrx_p = new Pin( 270, QPoint(-12, 16+8 ), id+"-vrx_p", 0, this );
    m_vrx_p->setLabelText( " XP" );
    m_pin[0] = m_vrx_p;

    m_vrx_m = new Pin( 270, QPoint(-4, 16+8 ), id+"-vrx_m", 0, this );
    m_vrx_m->setLabelText( " XM" );
    m_pin[1] = m_vrx_m;

    m_vry_p = new Pin( 270, QPoint( 4, 16+8 ), id+"-vry_p", 0, this );
    m_vry_p->setLabelText( " Yp" );
    m_pin[2] = m_vry_p;

    m_vry_m = new Pin( 270, QPoint( 12, 16+8 ), id+"-vry_m", 0, this );
    m_vry_m->setLabelText( " YM" );
    m_pin[3] = m_vry_m;

    m_resXA.setEpin( 0, m_vrx_p );
    m_resXA.setEpin( 1, &m_ePinXA );
    m_resXB.setEpin( 1, m_vrx_m );
    m_resXB.setEpin( 0, &m_ePinXB );
    m_resYA.setEpin( 0, m_vry_p );
    m_resYA.setEpin( 1, &m_ePinYA );
    m_resYB.setEpin( 0, m_vry_m );
    m_resYB.setEpin( 1, &m_ePinYB );
    m_resTouch.setEpin( 0, &m_ePinTA );
    m_resTouch.setEpin( 1, &m_ePinTB );

    m_width  = 240;
    m_height = 320;

    m_proxy = Circuit::self()->addWidget( &m_touchpadW );
    m_proxy->setParentItem( this );
    updateSize();

    setZValue( 10 );
    setShowId( true );

    Simulator::self()->addToUpdateList( this );

    addPropGroup( { tr("Main"), {
new IntProp <TouchPad>("Width", tr("Width")    ,"_Pixels", this, &TouchPad::width      , &TouchPad::setWidth ,0,"uint" ),
new IntProp <TouchPad>("Height",tr("Height")   ,"_Pixels", this, &TouchPad::height     , &TouchPad::setHeight ,0,"uint" ),
new BoolProp<TouchPad>("Transparent",tr("Transparent"),"", this, &TouchPad::transparent, &TouchPad::setTransparent )
    }, groupNoCopy} );
    addPropGroup( { tr("Electric"), {
new DoubProp<TouchPad>("RxMin", tr("RxMin"),"Ω", this, &TouchPad::RxMin, &TouchPad::setRxMin,0,"uint" ),
new DoubProp<TouchPad>("RxMax", tr("RxMax"),"Ω", this, &TouchPad::RxMax, &TouchPad::setRxMax,0,"uint" ),
new DoubProp<TouchPad>("RyMin", tr("RyMin"),"Ω", this, &TouchPad::RyMin, &TouchPad::setRyMin,0,"uint" ),
new DoubProp<TouchPad>("RyMax", tr("RyMax"),"Ω", this, &TouchPad::RyMax, &TouchPad::setRyMax,0,"uint" ),
    }, groupNoCopy} );
}
TouchPad::~TouchPad(){}

void TouchPad::initialize()
{
    m_eNodeX = new eNode( m_id+"-eNodeX" );
    m_eNodeY = new eNode( m_id+"-eNodeY" );
}

void TouchPad::stamp()
{
    m_ePinXA.setEnode( m_eNodeX );  // Set eNode to internal eResistors ePins
    m_ePinXB.setEnode( m_eNodeX );
    m_ePinYA.setEnode( m_eNodeY );
    m_ePinYB.setEnode( m_eNodeY );
    m_ePinTA.setEnode( m_eNodeX );
    m_ePinTB.setEnode( m_eNodeY );

    m_xPos = 0;
    m_yPos = 0;
    m_touchpadW.resetValues();
    updateStep();
}

void TouchPad::updateStep()
{
    int xPos = m_touchpadW.getXValue();
    int yPos = m_touchpadW.getYValue();

    if( m_xPos != xPos )
    {
        m_xPos = xPos;
        double xResA, xResB, tAdmit;

        if( xPos < 0 )       // Not touching
        {
            xResA = xResB = (m_RxMax-m_RxMin)/2;
            tAdmit = 0;
        }else{
            xResA = m_RxMin + (m_RxMax-m_RxMin)*xPos/m_width;
            xResB = m_RxMin + m_RxMax - xResA;
            tAdmit = 1;
        }
        m_resXA.setRes( xResA );
        m_resXB.setRes( xResB );
        m_resTouch.setAdmit( tAdmit );
        //qDebug()<<"X" << xResA<< xResB<< tAdmit;
    }
    if( m_yPos != yPos )
    {
        m_yPos = yPos;
        double yResA, yResB, tAdmit;

        if( xPos < 0 )       // Not touching
        {
            yResA = yResB = (m_RyMax-m_RyMin)/2;
            tAdmit = 0;
        }else{
            yResA = m_RyMin + (m_RyMax-m_RyMin)*yPos/m_height;
            yResB = m_RyMin + m_RyMax - yResA;
            tAdmit = 1;
        }
        m_resYA.setRes( yResA );
        m_resYB.setRes( yResB );
        m_resTouch.setAdmit( tAdmit );
        //qDebug()<<"y" << yResA<< yResB<< tAdmit;
    }
}

void TouchPad::setWidth( int w )
{
    if( w < 40 ) w = 40;
    m_width = w;
    updateSize();
}

void TouchPad::setHeight( int h )
{
    if( h < 20 ) h = 20;
    m_height = h;
    updateSize();
}

void TouchPad::setRxMin( double min )
{
    if( min > m_RxMax ) return;
    if( min < 1e-3 ) min = 1e-3;
    m_RxMin = min;
}

void TouchPad::setRxMax( double max )
{
    if( max < m_RxMin ) return;
    m_RxMax = max;
}

void TouchPad::setRyMin( double min )
{
    if( min > m_RyMax ) return;
    if( min < 1e-3 ) min = 1e-3;
    m_RyMin = min;
}

void TouchPad::setRyMax( double max )
{
    if( max < m_RyMin ) return;
    m_RyMax = max;
}

void TouchPad::updateSize()
{
    m_touchpadW.setFixedSize( m_width, m_height );
    m_proxy->setPos( QPoint(-m_width/2, -m_height-2) );

    m_area = QRect(-m_width/2,-m_height, m_width, m_height+18 );
    setLabelPos(-35, -m_height-15, 0);
}

void TouchPad::setTransparent( bool t )
{
    m_transparent = t;
    m_touchpadW.setAttribute( Qt::WA_NoSystemBackground, t );
    update();
}

QPainterPath TouchPad::shape() const
{
    QPainterPath path;
    QVector<QPointF> points;

    points << QPointF(-m_width/2,-m_height )
           << QPointF( m_width/2,-m_height )
           << QPointF( m_width/2, 0 )
           << QPointF( 42, 0  )
           << QPointF( 42, 18 )
           << QPointF(-10, 18 )
           << QPointF(-10, 0 )
           << QPointF(-m_width/2, 0  );

    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void TouchPad::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( Qt::transparent );
    p->drawRoundedRect( QRect(-m_width/2,-m_height-2, m_width, m_height ), 2, 2 );

    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( QRect(-20,-2, 40, 18 ), 2, 2 );
}

