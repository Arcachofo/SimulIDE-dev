/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
#pragma once

#include "picinterrupt.h"

class Pic14eInterrupt : public PicInterrupt
{
    public:
        Pic14eInterrupt( QString name, uint16_t vector, eMcu* mcu );
        ~Pic14eInterrupt();

        virtual void execute() override;
        virtual void exitInt() override;
};
