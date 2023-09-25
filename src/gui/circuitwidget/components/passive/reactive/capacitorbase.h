/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CAPACITORBASE_H
#define CAPACITORBASE_H

#include "reactive.h"

class MAINMODULE_EXPORT CapacitorBase : public Reactive
{
    public:
        CapacitorBase( QString type, QString id );
        ~CapacitorBase();

        virtual void setCurrentValue( double c ) override;

    protected:
        virtual double updtRes()  override { return m_tStep/m_capacitance; }
        virtual double updtCurr() override { return m_volt*m_admit; }

        double m_capacitance;
};

#endif
