/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#ifndef VARRESBASE_H
#define VARRESBASE_H

#include "e-resistor.h"
#include "comp2pin.h"
#include "dialwidget.h"

class MAINMODULE_EXPORT VarResBase : public Comp2Pin, public eResistor
{
    Q_OBJECT
    public:
        VarResBase( QObject* parent, QString type, QString id );
        ~VarResBase();

        double maxVal()  { return m_maxVal; }
        void setMaxVal( double max );

        double minVal() { return m_minVal; }
        void setMinVal( double min );

        double getVal() { return m_value; }
        void setVal( double val );

        double getStep() { return m_step; }
        void setStep( double step ) { if( step<0 ) step=0;m_step = step ; }

        virtual void initialize() override;


    public slots:
        void dialChanged( int val );

    protected:
        void updtValue();

        double m_value;
        double m_minVal;
        double m_maxVal;
        double m_step;

        DialWidget m_dialW;
        QDial*     m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif
