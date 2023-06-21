/***************************************************************************
 *   Copyright (C) 2020 by Benoit ZERR                                     *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#include <QPainter>
#include <iostream>

#include "strain.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"

#include "doubleprop.h"
#include "propdialog.h"

Component *Strain::construct (QObject* parent, QString type, QString id)
{ return new Strain (parent, type, id); }

LibraryItem* Strain::libraryItem()
{
    return new LibraryItem(
        tr( "Force Strain Gauge" ),
        "Resistive Sensors",
        "strain.png",
        "Strain",
        Strain::construct );
}

Strain::Strain( QObject* parent, QString type, QString id )
      : VarResBase( parent, type, id )
{
    m_area = QRectF( -12,-20, 24, 24 );

    setVal( 0 );  // start at 0 °C

    addPropGroup( { tr("Main"), {
new DoubProp<Strain>( "Force_N"    , tr("Current Value"),"N", this, &Strain::getVal,  &Strain::setVal ),
new DoubProp<Strain>( "Min_Force_N", tr("Minimum Value"),"N", this, &Strain::minVal,  &Strain::setMinVal ),
new DoubProp<Strain>( "Max_Force_N", tr("Maximum Value"),"N", this, &Strain::maxVal,  &Strain::setMaxVal ),
new DoubProp<Strain>( "Dial_Step"  , tr("Dial Step")    ,"N", this, &Strain::getStep, &Strain::setStep )
    }, 0} );
    addPropGroup( { tr("Parameters"), {
new DoubProp<Strain>( "Ref_Temp", tr("Ref. Temperature"),"ºC", this, &Strain::refTemp, &Strain::setRefTemp ),
new DoubProp<Strain>( "Temp"    , tr("Temperature")     ,"ºC", this, &Strain::getTemp, &Strain::setTemp )
    }, 0} );
    addPropGroup( { tr("Dial"), Dialed::dialProps(), groupNoCopy } );
}
Strain::~Strain() { }

void Strain::initialize()
{
  //m_last_step =  Simulator::self()->step();
  //m_resist = sensorFunction (m_sense);
  m_last_resist = 1/m_admit;
  m_new_resist = m_last_resist;

  //setResist ( m_resist);
  updateStep ();
}

void Strain::updateStep()
{
    if( !m_needUpdate ) return;
    m_needUpdate = false;

    /*m_step = Simulator::self()->step();
    double t = (double) m_step/1e6;
    double dt = t - m_t0_tau;

    m_new_resist = sensorFunction (m_sense);
    double dr = m_new_resist - m_last_resist;
    m_resist = m_last_resist + dr * (1.0 -exp(-dt/m_tau));
    
    setUnit (" ");
    setResist ( m_resist);*/

    //qDebug() << "Strain::Strain" << m_res.res() << m_res.current()
    //	     << m_ePinTst1->isConnected() << m_ePinTst2->isConnected() ;
    //m_last_step = m_step;

    double res = sensorFunction( m_value );
    eResistor::setRes( res );
    if( m_propDialog ) m_propDialog->updtValues();
    else if( m_showProperty == "Force_N" ) setValLabelText( getPropStr( "Force_N" ) );
}

void Strain::senseChanged( int val ) // Called when dial is rotated
{
    //qDebug() <<"Strain::senseChanged" << val;
    //double sense = (double)(m_dial->value());
    //sense = round(sense/(double)m_sense_step)*(double)m_sense_step;
    //qDebug()<<"Strain::senseChanged dialValue sense"<<m_dial->value()<<sense;
    //setSense( sense );
    //m_dialW.dial->setValue(m_sense);
    //m_t0_tau = (double) (Simulator::self()->step())/1e6;
    //m_last_resist = m_resist;
}


double Strain::sensorFunction(double forceN )
{
  double resisteance;
  m_k_long = 1+2.0*m_coef_poisson+m_cste_bridgman*(1-2*m_coef_poisson);
  //qDebug() << "K = " << m_k_long;
  m_k = m_k_long * (1 - m_coef_transverse);
  m_section_body = m_h_body * m_w_body;
  m_delta_r = m_r0*m_k*forceN/(m_young_modulus*m_section_body);
  m_delta_r_t = (m_alpha_r+m_k*(m_lambda_s - m_lambda_j)) * (m_temp - m_ref_temp) * m_r0;
  resisteance = m_r0 + m_delta_r + m_delta_r_t;
  return resisteance;
}

void Strain::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->drawRect(-11,-20, 22, 24 );
    p->fillRect(-8,-2, 4, 4, Qt::black );
    p->fillRect( 8,-2,-4, 4, Qt::black );

    QPen pen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );

    const QPointF points[8] = {
        QPointF(-6, 0 ),
        QPointF(-6,-16 ),
        QPointF(-2,-16 ),
        QPointF(-2,-4 ),
        QPointF( 2,-4 ),
        QPointF( 2,-16 ),
        QPointF( 6,-16 ),
        QPointF( 6, 0 ), };

    p->drawPolyline( points, 8 );
}
