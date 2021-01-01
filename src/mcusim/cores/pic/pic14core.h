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

#ifndef PIC14CORE_H
#define PIC14CORE_H

#include "corecpu.h"

class MAINMODULE_EXPORT Pic14Core : public CoreCpu
{
    public:
        Pic14Core( eMcu* mcu );
        ~Pic14Core();

        virtual void reset();
        virtual void runDecoder();

    protected:
        void createInstructions();

        inline void RETURN();
        inline void RETFIE();
        inline void OPTION();
        inline void SLEEP();
        inline void CLRWDT();
        inline void TRIS( uint8_t f );

        inline void MOVF( uint8_t f );
        inline void CLR( uint8_t f, uint8_t d );
};


#endif
