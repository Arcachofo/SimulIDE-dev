/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef REACTIVE_H
#define REACTIVE_H

#include "e-reactive.h"
#include "comp2pin.h"

class MAINMODULE_EXPORT Reactive : public Comp2Pin, public eReactive
{
    public:
        Reactive( QObject* parent, QString type, QString id );
        ~Reactive();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        double value() { return m_value; }
        void setValue( double c );

        double resist() { return m_resistor->res(); }
        void setResist( double resist );

        int autoStep() { return m_autoStep; }
        void setAutoStep( int a );

    private:
        eNode* m_midEnode;
        eResistor* m_resistor;
};

#endif
