/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRINTERRUPT_H
#define AVRINTERRUPT_H

#include "mcuinterrupts.h"
#include "mcutypes.h"

class MAINMODULE_EXPORT AVRInterrupt : public Interrupt
{
    public:
        AVRInterrupt( QString name, uint16_t vector, eMcu* mcu );
        ~AVRInterrupt();

 static Interrupt* getInterrupt( QString name, uint16_t vector, eMcu* mcu );

        //virtual void execute() override;
        //virtual void exitInt() override;

    private:
        //regBits_t m_I;
};

#endif
