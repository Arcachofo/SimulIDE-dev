/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef GATE_H
#define GATE_H

#include "logiccomponent.h"

class IoPin;

class MAINMODULE_EXPORT Gate : public LogicComponent
{
    public:
        Gate( QObject* parent, QString type, QString id, int inputs );
        ~Gate();

        QList<ComProperty*> edgeProps();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        bool initState() { return m_initState; }
        void setInitState( bool s) { m_initState = s; }

        void setNumInputs( int pins );

    protected:
        virtual bool calcOutput( int inputs );

        bool m_out;
        bool m_initState;

        int m_minInputs;
};

#endif
