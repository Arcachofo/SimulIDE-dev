/***************************************************************************
 *   Copyright (C) 2020 by Benoit ZERR                                     *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#ifndef STRAIN_H
#define STRAIN_H

#include "varresbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Strain : public VarResBase
{
 public:
    Strain( QObject* parent, QString type, QString id );
    ~Strain();

    static Component* construct( QObject* parent, QString type, QString id );
    static LibraryItem* libraryItem();

    virtual void initialize() override;
    virtual void updateStep() override;

    double refTemp()              { return m_ref_temp; }
    void   setRefTemp( double t ) { m_ref_temp = t; }

    double getTemp()              { return m_temp; }
    void   setTemp( double t)     { m_temp= t; }
    
    //public slots:
      void senseChanged( int val );

      virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;
      
 private:
    double sensorFunction ( double forceN );

    uint64_t m_step;
    uint64_t m_last_step;

    double m_new_resist;
    double m_last_resist;

    double m_tau = 0.01;             // 10 ms (fast sensor)
    double m_t0_tau = 0.0;
    double m_r0 = 120.0;
    double m_coef_poisson = 0.3;     // 0.27 - 0.34 Steel - Alu
    double m_cste_bridgman = 1.3;    // around 1, 1.3 to get Constantan or Karma K=2.1
    double m_k_long;                 // gauge factor : perfect longitunal)
    double m_k;                      // gauge factor (include transverse decrease)
    double m_coef_transverse = 0.01; // to take into account non effectuve transverse wires
    double m_ref_temp = 20.0;        // ambient reference for thermal drift
    double m_temp = 20.0;            // operating temperature (here no thermal drift)
    double m_young_modulus = 69e9;   // Aluminum test body
    double m_h_body = 0.1e-3;        // rectangular section of test body , height=0.1 mm
    double m_w_body = 5e-3;          // width=5mm
    double m_section_body;
    double m_delta_r;                // resistance variation due to strain
    double m_alpha_r  = 2e-5;        // per °C for Constantan
    double m_lambda_j = 1.7e-5;      // per °C for Constantan
    double m_lambda_s = 2.5e-5;      // per °C for Aluminum body
    double m_delta_r_t;              // resisrance viariation due to thermal drift
};

#endif
