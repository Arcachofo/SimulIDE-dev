/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/
/*
 *   Based on simavr decoder
 *
 *   Copyright 2008, 2010 Michel Pollet <buserror@gmail.com>
 *
 */

#ifndef AVRCORE_H
#define AVRCORE_H

#include "mcucpu.h"

class MAINMODULE_EXPORT AvrCore : public McuCpu
{
    public:
        AvrCore( eMcu* mcu );
        ~AvrCore();

        //virtual void reset();
        virtual void runStep() override;

    private:
        uint16_t m_rampzAddr;
        uint8_t* RAMPZ;   // optional, only for ELPM/SPM on >64Kb cores
        uint8_t* EIND;    // optional, only for EIJMP/EICALL on >64Kb cores

        void flags_ns( uint8_t res );
        void flags_zns( uint8_t res );
        void flags_Rzns( uint8_t res );
        void flags_sub( uint8_t res, uint8_t rd, uint8_t rr );
        void flags_sub_Rzns( uint8_t res, uint8_t rd, uint8_t rr );
        void flags_add_zns( uint8_t res, uint8_t rd, uint8_t rr );
        void flags_sub_zns( uint8_t res, uint8_t rd, uint8_t rr );
        void flags_znv0s( uint8_t res );
        void flags_zcnvs( uint8_t res, uint8_t vr );
        void flags_zcvs( uint8_t res, uint8_t vr );
        void flags_zns16( uint16_t res );
        int  is_instr_32b( uint32_t pc );
};


#endif
