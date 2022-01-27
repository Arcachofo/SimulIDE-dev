/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef EDIODE_H
#define EDIODE_H

#include <QHash>

#include "e-resistor.h"

struct diodeData_t
{
    double satCur;
    double emCoef;
    double brkDow;
    double resist;
};

class eNode;

class MAINMODULE_EXPORT eDiode : public eResistor
{
    public:
        eDiode( QString id );
        ~eDiode();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void setRes( double r ) override { m_resistor->setRes( r ); }
        virtual void setResSafe( double r )  override { m_resistor->setResSafe( r ); }
        virtual double res() override { return m_resistor->res(); }

        virtual ePin* getEpin( int num ) override;
        virtual void  setEpin( int num, ePin* pin ) override;

        double threshold() { return m_fdDrop; }
        void   setThreshold( double fdDrop );

        double brkDownV(){ return m_bkDown; }
        void   setBrkDownV( double bkDown );

        double satCur() { return m_satCur; }
        void   setSatCur( double satCur );

        double emCoef() { return m_emCoef; }
        void   setEmCoef( double emCoef );

        double maxCurrent() { return m_maxCur; }
        void   setMaxCurrent( double cur ) { m_maxCur = cur; }

        double getCurrent() { return m_resistor->current(); }

        QString model() { return m_model; }
        void setModel( QString model );
        void setModelData( diodeData_t data );

        void createSerRes();

 static void getModels();

    protected:
        double limitStep( double vnew, double scale, double vc );
        void SetParameters( double sc, double ec, double bv, double sr );
        void updateValues();

        bool m_converged;

        double m_vt;
        double m_satCur;
        double m_emCoef;
        double m_vScale;
        double m_vdCoef;
        double m_vzCoef;
        double m_zOfset;
        double m_vCriti;
        double m_vzCrit;

        double m_fdDrop;
        double m_bkDown;
        double m_maxCur;

        double m_voltPN;
        double m_bAdmit;

        ePin* m_pinP;
        ePin* m_pinN;
        ePin* m_pinR0;
        ePin* m_pinR1;
        eNode* m_midEnode;
        eResistor* m_resistor;

        QString m_diodeType;
        QString m_model;

 static QHash<QString, diodeData_t> m_diodes;
 static QHash<QString, diodeData_t> m_zeners;
 static QHash<QString, diodeData_t> m_leds;
};

#endif
