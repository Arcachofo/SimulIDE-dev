/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DECTOBCD_H
#define DECTOBCD_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT DecToBcd : public LogicComponent
{
    public:
        DecToBcd( QObject* parent, QString type, QString id );
        ~DecToBcd();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        bool is16Bits() { return m_16Bits; }
        void set16bits( bool set );

    private:
        bool m_16Bits;
        int  m_bits;
};

#endif

