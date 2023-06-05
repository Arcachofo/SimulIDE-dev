/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QtMath>

#include "thermistor.h"
#include "itemlibrary.h"

#include "intprop.h"
#include "propdialog.h"

Component* Thermistor::construct( QObject* parent, QString type, QString id )
{ return new Thermistor( parent, type, id ); }

LibraryItem* Thermistor::libraryItem()
{
    return new LibraryItem(
        tr( "Thermistor" ),
        "Resistive Sensors",
        "thermistor.png",
        "Thermistor",
        Thermistor::construct);
}

Thermistor::Thermistor( QObject* parent, QString type, QString id )
          : ThermistorBase( parent, type, id  )
{
    m_bVal = 3455;
    m_r25  = 10000;

    addPropGroup( { tr("Parameters"), {
new IntProp<Thermistor>( "B"  , "B"  ,""  , this, &Thermistor::bVal, &Thermistor::setBval, "uint" ),
new IntProp<Thermistor>( "R25", "R25","Ω" , this, &Thermistor::r25,  &Thermistor::setR25,  "uint" )
    }} );
}
Thermistor::~Thermistor(){}

void Thermistor::updateStep()
{
    if( !m_changed ) return;
    m_changed = false;

    double t0 = 25+273.15;      // Temp in Kelvin
    double t = m_value+273.15;
    double e = 2.7182;
    //double k = t*t0/(t-t0);
    //double res = m_r25/pow( e, m_bVal/k );
    double k = (t0-t)/(t*t0);
    double res = m_r25*qPow( e, m_bVal*k );
    eResistor::setRes( res );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Temp" ) setValLabelText( getPropStr( "Temp" ) );
}

void Thermistor::setBval( int bval )
{
    m_bVal = bval;
    m_changed = true;
}

void Thermistor::setR25( int r25 )
{
    m_r25 = r25;
    m_changed = true;
}

void Thermistor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    Component::paint( p, option, widget );

    p->drawRect( QRectF(-10.5, -4, 21, 8 ));
    
    p->drawLine(-8, 6,  6,-8 );
    p->drawLine( 6,-8, 10,-8 );
}
