/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FULLADDER_H
#define FULLADDER_H

#include "iocomponent.h"
#include "e-element.h"

class LibraryItem;

class FullAdder : public IoComponent, public eElement
{
    public:
        FullAdder( QString type, QString id );
        ~FullAdder();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        virtual void remove() override;

        int bits() { return m_bits; }
        void setBits( int b );

    private:

        int m_bits;

        IoPin* m_ciPin;
        IoPin* m_coPin;

        std::vector<IoPin*> m_inA;
        std::vector<IoPin*> m_inB;
        std::vector<IoPin*> m_outS;
};

#endif
