/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
/*
 *   Based on simavr decoder
 *
 *   Copyright 2008, 2010 Michel Pollet <buserror@gmail.com>
 *
 */

#ifndef AVRCORE_H
#define AVRCORE_H

#include "corecpu.h"


class MAINMODULE_EXPORT AvrCore : public CoreCpu
{
    public:
        AvrCore( eMcu* mcu );
        ~AvrCore();

        virtual void reset();
        virtual void runDecoder();

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
