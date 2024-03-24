/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BCDBASE_H
#define BCDBASE_H

#include "logiccomponent.h"

class BcdBase : public LogicComponent
{
    public:
        BcdBase( QString type, QString id );
        ~BcdBase();

        bool tristate() { return true; }

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

    protected:
 static const uint8_t m_values[];

    uint8_t m_digit;
};

#endif
