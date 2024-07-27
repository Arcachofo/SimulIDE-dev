/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BINCOUNTER_H
#define BINCOUNTER_H

#include "logiccomponent.h"

class LibraryItem;

class BinCounter : public LogicComponent
{
    public:
        BinCounter( QString type, QString id );
        ~BinCounter();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        bool srInv() { return m_resetInv; }
        void setSrInv( bool inv );

        bool pinSet() { return m_pinSet; }
        void useSetPin( bool set );

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        int maxVal() { return m_TopValue; }
        void setMaxVal( int v ) { m_TopValue = v; }

    protected:
        int m_Counter;
        int m_TopValue;

        bool m_resetInv;
        bool m_pinSet;

        IoPin* m_setPin;
        IoPin* m_rstPin;
};

#endif
