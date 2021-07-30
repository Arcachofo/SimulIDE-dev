/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
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

#ifndef IOPIN_H
#define IOPIN_H

#include <QColor>

#include "pin.h"
#include "e-node.h"
#include "e-element.h"

enum pinMode_t{
    undef_mode=0,
    input,
    output,
    open_col,
    source
};

class MAINMODULE_EXPORT IoPin : public Pin, public eElement
{
    public:
        IoPin( int angle, const QPoint pos, QString id, int index, Component* parent, pinMode_t mode=source );
        ~IoPin();

        virtual void initialize() override;
        virtual void stamp() override { stampAll(); }

        void stampAll() { ePin::stampAdmitance( m_admit ); stampOutput(); }
        void stampOutput() { m_scrEnode->setVolt( m_outVolt ); ePin::stampCurrent( m_outVolt/m_imp ); }

        pinMode_t pinMode() { return m_pinMode; }
        void setPinMode( pinMode_t mode );

        void  setInputHighV( double volt ) { m_inpHighV = volt; }
        void  setInputLowV( double volt ) { m_inpLowV = volt; }
        virtual void  setInputImp( double imp );

        double outHighV() { return m_outHighV; }
        void  setOutHighV( double v ) { m_outHighV = v; }

        double outLowV() { return m_outLowV; }
        void  setOutLowV( double v ) { m_outLowV = v; }

        virtual void  setOutputImp( double imp );

        double imp() { return m_imp; }
        virtual void  setImp( double imp );

        virtual bool getInpState();
        virtual bool getOutState() { return m_outState; }
        virtual void setOutState( bool out, bool st=true );
        virtual void toggleOutState() { IoPin::setOutState( !m_outState ); }

        void setStateZ( bool z );

        virtual void setInverted( bool inverted ) override;

        virtual void controlPin( bool outCtrl , bool dirCtrl );

    protected:
        void updtState();

        double m_inpHighV;  // currently in eClockedDevice
        double m_inpLowV;

        double m_outHighV;
        double m_outLowV;
        double m_outVolt;

        double m_vddAdmit;  // Out stage
        double m_gndAdmit;  // Out Stage
        double m_vddAdmEx;  // Extra Source (vref out)
        double m_gndAdmEx;  // Extra Source (vref out)

        double m_inputImp;
        double m_outputImp;
        double m_openImp;
        double m_imp;
        double m_admit;

        bool m_inpState;
        bool m_outState;
        bool m_stateZ;
        bool m_outCtrl;
        bool m_dirCtrl;

        pinMode_t m_pinMode;
        pinMode_t m_oldPinMode;

        eNode* m_scrEnode;
};
#endif
