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
        CapacitorBase( QObject* parent, QString type, QString id );
        ~CapacitorBase();

    protected:
        virtual double updtRes()  override { return m_tStep/m_value; }
        virtual double updtCurr() override { return m_volt*m_admit; }
};

#endif
