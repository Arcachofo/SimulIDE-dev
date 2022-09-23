/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef Pic14eInterrupt_H
#define Pic14eInterrupt_H

#include "picinterrupt.h"

class MAINMODULE_EXPORT Pic14eInterrupt : public PicInterrupt
{
    public:
        Pic14eInterrupt( QString name, uint16_t vector, eMcu* mcu );
        ~Pic14eInterrupt();

        virtual void execute() override;
        virtual void exitInt() override;

    protected:
        uint8_t* m_wReg;
        uint8_t* m_status;
        uint8_t* m_bsr;
        uint8_t* m_pclath;

        uint8_t m_wRegSaved;
        uint8_t m_statusSaved;
        uint8_t m_bsrSaved;
        uint8_t m_pclathSaved;
};

#endif
