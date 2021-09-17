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

#include "e-resistor.h"

class eNode;

class MAINMODULE_EXPORT eDiode : public eResistor
{
    public:
        eDiode( QString id );
        ~eDiode();

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;

        virtual void   setRes( double resist )  override { m_resistor->setResSafe( resist ); }
        virtual double res() override { return m_resistor->res(); }

        virtual ePin* getEpin( int num ) override;
        virtual void  setEpin( int num, ePin* pin ) override;

        double threshold() { return m_fdDrop; }
        void   setThreshold( double fdDrop );

        double brkDownV(){ return m_bkDown; }
        void   setBrkDownV( double bkDown );

        double satCur_nA() { return m_satCur*1e9; }
        void   setSatCur_nA( double sc_nA ) { setSatCur( sc_nA*1e-9 ); }

        double satCur() { return m_satCur; }
        void   setSatCur( double satCur );

        double emCoef() { return m_emCoef; }
        void   setEmCoef( double emCoef );

    protected:
        double limitStep( double vnew, double scale, double vc );
        void SetParameters( double sc, double ec, double bv, double sr );
        void updateValues();
        void createSerRes();

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

        double m_voltPN;
        double m_bAdmit;

        ePin* m_pinP;
        ePin* m_pinN;
        ePin* m_pinR0;
        ePin* m_pinR1;
        eNode* m_midEnode;
        eResistor* m_resistor;
};
#endif


