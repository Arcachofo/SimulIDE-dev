/***************************************************************************
 *   Copyright (C) 2020 by Benoit ZERR                                       *
 *                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
/*
 *   Modified 2020 by santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */


#include <iostream>

#include "strain.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"


static const char* Strain_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Value Ohm")
};

Component *Strain::construct (QObject* parent, QString type, QString id)
{
  return new Strain (parent, type, id);
}

LibraryItem* Strain::libraryItem()
{
    return new LibraryItem(
        tr( "Force Strain Gauge" ),
        tr( "Passive" ),
        "strain.png",
        "Strain",
        Strain::construct );
}

Strain::Strain (QObject* parent, QString type, QString id)
      : VarResBase( parent, type, id )

  {
    Q_UNUSED( Strain_properties );
    m_area = QRectF( -12,-20, 24, 24 );

    m_unit = "N";

    m_dial->setMinimum(0);
    m_dial->setMaximum(250);
    setVal( 0 );  // start at 0 °C
  }

Strain::~Strain() { }

void Strain::initialize()
{
  //m_last_step =  Simulator::self()->step();
  //m_resist = sensorFunction (m_sense);
  m_last_resist = m_resist;
  m_new_resist = m_resist;
  //setUnit (" ");
  //setResist ( m_resist);
  updateStep ();
}

void Strain::updateStep()
{
    if( !m_changed ) return;
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
    eResistor::setResSafe( res );
    m_changed = false;
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


double Strain::sensorFunction( double sense )
{
  double r_sense;
  m_k_long = 1+2.0*m_coef_poisson+m_cste_bridgman*(1-2*m_coef_poisson);
  //qDebug() << "K = " << m_k_long;
  m_k = m_k_long * (1 - m_coef_transverse);
  m_section_body = m_h_body * m_w_body;
  m_delta_r = m_r0*m_k*sense/(m_young_modulus*m_section_body);
  m_delta_r_t = (m_alpha_r+m_k*(m_lambda_s - m_lambda_j)) * (m_temp - m_ref_temp) * m_r0;
  r_sense = m_r0 + m_delta_r + m_delta_r_t;
  return r_sense;
}

void Strain::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->drawRect( -10,-20, 20, 24 );
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

#include "moc_strain.cpp"
