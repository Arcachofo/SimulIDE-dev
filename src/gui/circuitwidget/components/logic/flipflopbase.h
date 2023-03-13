/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FLIPFLOPBASE_H
#define FLIPFLOPBASE_H

#include "logiccomponent.h"

class MAINMODULE_EXPORT FlipFlopBase : public LogicComponent
{
    public:
        FlipFlopBase( QObject* parent, QString type, QString id );
        ~FlipFlopBase();

        bool srInv() { return m_srInv; }
        void setSrInv( bool inv );

        bool pinsRS() { return m_useRS; }
        void usePinsRS( bool rs );

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        bool sPinState();
        bool rPinState();

    protected:
        virtual void calcOutput(){;}
        bool m_useRS;
        bool m_srInv;
        bool m_Q0;

        int m_dataPins;

        IoPin* m_setPin;
        IoPin* m_resetPin;
};

#endif
