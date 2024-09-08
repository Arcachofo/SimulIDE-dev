/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QGraphicsProxyWidget>
#include <QDial>
#include <QPainter>

#include "varsource.h"
#include "simulator.h"
#include "circuit.h"
#include "custombutton.h"

#include "boolprop.h"

#define WIDTH 40
#define HEIGHT 56
#define DIAL_SIZE 36

VarSource::VarSource( QString type, QString id )
         : Component( type, id )
         , eElement( id )
{
    m_area = QRect( -WIDTH/2, -HEIGHT/2, WIDTH, HEIGHT );

    m_graphical = true;
    m_changed = false;
    m_minValue = 0.0;
    m_outValue = 5.0;

    m_voltw.setFixedSize( WIDTH-4, HEIGHT-4 );
    
    m_proxy = Circuit::self()->addWidget( &m_voltw );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-WIDTH/2+2, -HEIGHT/2+2 ) );
    //m_proxy->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true );

    m_button = m_voltw.pushButton;
    m_button->setFixedSize( WIDTH-8,16 );

    setVal( 0 );

    setValLabelPos(-8, 30 , 0 ); // x, y, rot
    setLabelPos(-32,-48, 0 );

    Simulator::self()->addToUpdateList( this );

    QObject::connect( m_button, &CustomButton::clicked, [=](){ onbuttonclicked(); } );
    QObject::connect( m_voltw.dial(), &QDial::valueChanged, [=](int v){ dialChanged(v); } );

    addPropGroup( { "Hidden1", {
        new BoolProp<VarSource>("Running","",""
                               , this, &VarSource::running, &VarSource::setRunning ),
    }, groupHidden} );
}
VarSource::~VarSource() { }

void VarSource::initialize(){ m_changed = true; }

void VarSource::updateButton()
{
    QString msg;
    bool checked = m_button->isChecked();
    
    if( checked )
          msg = QString::number( m_outValue,'f', 2 ).left(5)+" "+m_unit;
    else  msg = QString("---"+m_unit);
        
    m_button->setText( msg );
}

void VarSource::onbuttonclicked()
{
    updateButton();
    m_changed = true;
}

void VarSource::dialChanged( int val )
{
    m_outValue = m_minValue+(m_maxValue-m_minValue)*double(val)/1000.0;
    updateButton();
    m_changed = true;
}

void VarSource::setVal( double val )
{
    if     ( val > m_maxValue ) m_maxValue = val;
    else if( val < m_minValue ) m_minValue = val;

    m_voltw.setValue( val*1000/(m_maxValue-m_minValue) );
    m_outValue = val;
    m_changed = true;
    updateButton();
    if( !Simulator::self()->isRunning() ) updateStep();
}

void VarSource::setMaxValue( double v )
{
    if( v < m_minValue ) v = m_minValue+1e-3;
    m_maxValue = v;
    if( m_outValue > v ) m_outValue = v;
    m_voltw.setValue( m_outValue*1000/(m_maxValue-m_minValue) );
    m_changed = true;
}

void VarSource::setMinValue( double v )
{
    if( v > m_maxValue ) v = m_maxValue-1e-3;
    m_minValue = v;
    if( m_outValue < v ) m_outValue = v;
    m_voltw.setValue( m_outValue*1000/(m_maxValue-m_minValue) );
    m_changed = true;
}

void VarSource::setflip()
{
    Component::setflip();
    m_proxy->setPos( QPoint( (-WIDTH/2+2)*m_Hflip, (-HEIGHT/2+2)*m_Vflip ) );
    m_proxy->setTransform( QTransform::fromScale( m_Hflip, m_Vflip ) );
}

bool VarSource::running() { return m_button->isChecked(); }
void VarSource::setRunning( bool r ) { m_button->setChecked( r ); }

void VarSource::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    Component::paint( p, o, w );

    p->setBrush(QColor( 230, 230, 230 ));
    p->drawRoundedRect( m_area, 2, 2 );

    //p->fillRect( m_area, Qt::darkGray );

    Component::paintSelected( p );
}
