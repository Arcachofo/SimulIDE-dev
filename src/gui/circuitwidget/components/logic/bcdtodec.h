/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BCDTODEC_H
#define BCDTODEC_H

#include "logiccomponent.h"

class LibraryItem;

class BcdToDec : public LogicComponent
{
    public:
        BcdToDec( QString type, QString id );
        ~BcdToDec();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        bool is16Bits() { return m_16Bits; }
        void set_16bits( bool set );

   private:
        bool m_16Bits;
};

#endif

