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
