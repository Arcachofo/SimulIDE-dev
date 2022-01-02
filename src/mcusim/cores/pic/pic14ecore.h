/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef PIC14ECORE_H
#define PIC14ECORE_H

#include "picmrcore.h"

class MAINMODULE_EXPORT Pic14eCore : public PicMrCore
{
    public:
        Pic14eCore( eMcu* mcu );
        ~Pic14eCore();

        //virtual void reset();

    protected:
        virtual void runDecoder( uint16_t instr ) override;
        virtual void setBank( uint8_t bank ) override { PicMrCore::setBank( bank ); }

        // Miscellaneous instructions
        //inline void RESET();
        inline void CALLW();
        inline void BRW();
        inline void MOVIW_pF( uint8_t n );
        inline void MOVIW_nF( uint8_t n );
        inline void MOVIW_Fp( uint8_t n );
        inline void MOVIW_Fn( uint8_t n );
        inline void MOVWI_pF( uint8_t n );
        inline void MOVWI_nF( uint8_t n );
        inline void MOVWI_Fp( uint8_t n );
        inline void MOVWI_Fn( uint8_t n );
        inline void MOVLB( uint8_t k );

        // ALU operations: dest ← OP(f,W)
        inline void LSLF( uint8_t f, uint8_t d );
        inline void LSRF( uint8_t f, uint8_t d );
        inline void ASRF( uint8_t f, uint8_t d );
        inline void SUBWFB( uint8_t f, uint8_t d );
        inline void ADDWFC( uint8_t f, uint8_t d );

        // Operations with literal k
        inline void ADDFSR( uint8_t n, uint8_t k );
        inline void MOVLP( uint8_t k );
        inline void BRA( uint8_t k );
        inline void MOVIW( uint8_t n, uint8_t k );
        inline void MOVWI( uint8_t n, uint8_t k );
};
#endif
