/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SHIFTREG_H
#define SHIFTREG_H

#include "logiccomponent.h"

class LibraryItem;

class ShiftReg : public LogicComponent
{
    public:
        ShiftReg( QString type, QString id );
        ~ShiftReg();

 static Component* construct( QString type, QString id );
 static LibraryItem *libraryItem();

        virtual bool setPropStr( QString prop, QString val ) override;

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        int bits() { return m_bits; }
        void setBits( int b );

        bool parallelIn() { return m_parallelIn; }
        void setParallelIn( bool p );

        bool bidirectional() { return m_bidir; }
        void setBidirectional( bool b );

    private:
        void updatePins();

        int m_bits;

        bool m_parallelIn;
        bool m_bidir;
        bool m_ldInps;

        IoPin* m_dinPin;
        IoPin* m_dilPin;
        IoPin* m_dirPin;
        IoPin* m_rstPin;
        IoPin* m_serPin;
};

#endif
