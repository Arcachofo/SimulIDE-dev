/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PicINTERRUPT_H
#define PicINTERRUPT_H

#include "mcuinterrupts.h"
#include "mcutypes.h"

class MAINMODULE_EXPORT PicInterrupt : public Interrupt
{
    public:
        PicInterrupt( QString name, uint16_t vector, eMcu* mcu );
        ~PicInterrupt();

        //virtual void execute() override;
        //virtual void exitInt() override;

    private:
        //regBits_t m_GIE;
};

#endif
