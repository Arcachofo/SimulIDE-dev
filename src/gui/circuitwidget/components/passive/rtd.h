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


#ifndef RTD_H
#define RTD_H

#include "thermistorbase.h"

class LibraryItem;

class MAINMODULE_EXPORT RTD : public ThermistorBase
{
    Q_OBJECT
    Q_PROPERTY( double R0      READ getR0   WRITE setR0   DESIGNABLE true USER true )
    //Q_PROPERTY( bool   Show_sense READ showSense  WRITE setShowSense DESIGNABLE true USER true )

 public:
    RTD( QObject* parent, QString type, QString id );
    ~RTD();

    static Component* construct( QObject* parent, QString type, QString id );
    static LibraryItem* libraryItem();

    virtual QList<propGroup_t> propGroups() override;

    double getR0() { return m_r0; }
    void setR0( double r0 ) { m_r0 = r0; }

    virtual void initialize() override;
    virtual void updateStep() override;

    double sensorFunction ( double temp );

    virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

 private:
    uint64_t m_lastTime;

    double m_tau = 0.3;  // 0.3 in water 4.0 in air
    double m_t0_tau = 0.0;
    double m_r0 = 100.0;
    //double coef_temp = 0.00385;  // linear approximation
    //here we use a more realist third order polynomial approximation
    double coef_temp_a =3.9083e-3;  // a * T
    double coef_temp_b = -5.775e-7; // b*T2
    double coef_temp_c =-4.183e-12;  // c*(T-100)*T3 (T < 0 only)
};

#endif
