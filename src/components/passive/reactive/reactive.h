/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef REACTIVE_H
#define REACTIVE_H

#include "e-reactive.h"
#include "comp2pin.h"

class Reactive : public Comp2Pin, public eReactive
{
    public:
        Reactive( QString type, QString id );
        ~Reactive();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        double value() { return m_value; }
        void setValue( double c );

        double resist() { return m_resistor->resistance(); }
        void setResist( double resist );

        double reaStep() { return m_reacStep*1e-12; }
        void setReaStep( double r );

        virtual void setCurrentValue( double c ){;}

        virtual void setLinkedValue( double v, int i=0 ) override;

    private:
        eNode* m_midEnode;
        eResistor* m_resistor;
};

#endif
